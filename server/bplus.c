#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include "bplus.h"

pager_t* open_pager(char* filename) {
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        return NULL;
    }
    pager_t* pager = (pager_t*)calloc(1, sizeof(pager_t));
    pager->fd = fd;
    pager->size = lseek(fd, 0, SEEK_END);
    if (pager->size % PAGE_SIZE != 0) {
        write(2, ERR_CORRUPT, sizeof(ERR_CORRUPT));
        exit(EXIT_FAILURE);
    }
    pager->page_num = pager->size / PAGE_SIZE;
    return pager;
}

// add page to pager in memory
page_t* add_page(pager_t* pager, int is_leave, uint32_t parent) {
    page_t* page = (page_t*)malloc(sizeof(page_t));
    page_meta meta = {is_leave, 1, 0, pager->page_num, parent};
    page->meta = meta;
    pager->pages[pager->page_num] = page;
    pager->page_num++;
    return page;
}

// read page to pager from disk
page_t* read_page(pager_t* pager, int page_num) {
    if (pager->pages[page_num] == NULL) {  // ensure that page donot be read before
        pager->pages[page_num] = (page_t*)malloc(sizeof(page_t));
        lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);
        read(pager->fd, pager->pages[page_num], PAGE_SIZE);
    }
    return pager->pages[page_num];
}

void flush_page(pager_t* pager, int page_num) {
    pager->pages[page_num]->meta.changed = 0;
    lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);
    write(pager->fd, pager->pages[page_num], PAGE_SIZE);
}

void flush_pages(pager_t* pager) {
    for (int i = 0, n = pager->page_num; i < n; i++) {
        if (pager->pages[i] != NULL && pager->pages[i]->meta.changed) {
            pager->pages[i]->meta.changed = 0;
            lseek(pager->fd, i * PAGE_SIZE, SEEK_SET);
            write(pager->fd, pager->pages[i], PAGE_SIZE);
            printf("write at page %d\n", i);
        }
    }
}

table_t* open_db(char* filename) {
    pager_t* pager = open_pager(filename);
    if (pager == NULL) {
        return NULL;
    }
    table_t* table = (table_t*)malloc(sizeof(table_t));
    table->meta = (table_meta*)malloc(sizeof(table_meta));
    table->pager = pager;
    // ensure root page is loaded in memory
    if (pager->page_num < 2) {
        table->meta->root_page_num = 1;
        pager->page_num = 1;
        add_page(pager, 1, 0);
        ((leave_t*)pager->pages[1])->next = 0;
    } else {
        lseek(pager->fd, 0, SEEK_SET);
        read(pager->fd, table->meta, PAGE_SIZE);
        read_page(pager, table->meta->root_page_num);
    }
    return table;
}

void close_db(table_t* table) {
    lseek(table->pager->fd, 0, SEEK_SET);
    write(table->pager->fd, table->meta, PAGE_SIZE);
    flush_pages(table->pager);
    close(table->pager->fd);
}

int bi_search(void* array, size_t num, size_t size, void* target, int (*cmp)(void*, void*)) {
    int begin = 0, end = num;
    while (begin < end) {
        int mid = (begin + end - 1) / 2;
        if (cmp(target, array + mid * size)) {
            end = mid;
        } else {
            begin = mid + 1;
        }
    }
    return begin;
}

int key_cmp(key_t* key1, key_t* key2) {
    return key1->key < key2->key;
}

int insert(table_t* table, key_t key, row_t value) {
    page_t* cur_page = read_page(table->pager, table->meta->root_page_num);
    while (!cur_page->meta.is_leave) {
        internal_t* internal = (internal_t*)cur_page;
        int pos = bi_search(internal->keys, internal->meta.key_num, sizeof(key_t), &key, key_cmp);
        if (memcmp(internal->keys + pos, &key, sizeof(key_t)) == 0) {
            printf("duplicate primary key\n");
            return -1;
        }
        int page_num = internal->pointers[pos];
        cur_page = read_page(table->pager, page_num);
    }
    cur_page->meta.changed = 1;
    leave_t* leave = (leave_t*)cur_page;
    if (leave->meta.key_num == LEAVE_PAIR_NUM) {
        int pos = bi_search(leave->keys, LEAVE_PAIR_NUM, sizeof(key_t), &key, key_cmp);
        if (leave->meta.parent == 0) {  // root node case
            internal_t* root = (internal_t*)add_page(table->pager, 0, 0);
            table->meta->root_page_num = root->meta.page_num;
            leave_t* brother = (leave_t*)add_page(table->pager, 1, root->meta.page_num);
            leave->meta.parent = root->meta.page_num;
            int i = LEAVE_PAIR_NUM / 2;
            root->keys[0] = pos == i ? key : leave->keys[i];
            root->meta.changed = 1;
            root->meta.key_num = 1;
            root->pointers[0] = leave->meta.page_num;
            root->pointers[1] = brother->meta.page_num;
            leave->meta.changed = 1;
            brother->meta.changed = 1;
            leave->meta.key_num = i + 1;
            brother->meta.key_num = LEAVE_PAIR_NUM - i;
            brother->next = leave->next;
            leave->next = brother->meta.page_num;
            if (pos > i) {
                memcpy(brother->keys, leave->keys + i + 1, sizeof(key_t) * (pos - i - 1));
                memcpy(brother->rows, leave->rows + i + 1, sizeof(row_t) * (pos - i - 1));
                brother->keys[pos - i - 1] = key;
                brother->rows[pos - i - 1] = value;
                memcpy(brother->keys + pos - i, leave->keys + pos + 1, sizeof(key_t) * (pos - i - 1));
                memcpy(brother->rows + pos - i, leave->rows + pos + 1, sizeof(row_t) * (pos - i - 1));
            } else {
                memcpy(brother->keys, leave->keys + i, sizeof(key_t) * (LEAVE_PAIR_NUM - i));
                memcpy(brother->rows, leave->rows + i, sizeof(row_t) * (LEAVE_PAIR_NUM - i));
                for (int j = i; j > pos; j--) {
                    leave->keys[j] = leave->keys[j - 1];
                    leave->rows[j] = leave->rows[j - 1];
                }
                leave->keys[pos] = key;
                leave->rows[pos] = value;
            }
        } else {
        }
    } else {
        int i = leave->meta.key_num;
        for (; key_cmp(&key, leave->keys + i - 1); i--) {
            leave->keys[i] = leave->keys[i - 1];
            leave->rows[i] = leave->rows[i - 1];
        }
        leave->keys[i] = key;
        leave->rows[i] = value;
        leave->meta.key_num++;
    }
    return 1;
}

void print_key(key_t key) {
    printf("%d", key.key);
}

void print_row(row_t row) {
    printf("(%d, %s, %s)", row.id, row.username, row.email);
}

void print_table(table_t* table) {
    flush_pages(table->pager);
    page_t* cur_page = read_page(table->pager, table->meta->root_page_num);
    while (cur_page->meta.key_num > 0 && !cur_page->meta.is_leave) {
        cur_page = read_page(table->pager, ((internal_t*)cur_page)->pointers[0]);
    }
    while (1) {
        for (int i = 0; i < cur_page->meta.key_num; i++) {
            printf("key:");
            print_key(((leave_t*)cur_page)->keys[i]);
            printf(", row:");
            print_row(((leave_t*)cur_page)->rows[i]);
            printf("\n");
        }
        if (((leave_t*)cur_page)->next == 0) {
            break;
        }
        cur_page = read_page(table->pager, ((leave_t*)cur_page)->next);
    }
}

// Test start
#ifdef TEST

int cmp(int* a, int* b) {
    return *a < *b;
}

int main(int argc, char const* argv[]) {
    int test[] = {1, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    int target = 0;
    printf("%d\n", bi_search(test, sizeof(test) / sizeof(test[0]), sizeof(test[0]), &target, cmp));
    return 0;
}

#endif