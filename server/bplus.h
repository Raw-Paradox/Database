#define PAGE_SIZE 4096
#define MAX_PAGES 1024

typedef unsigned int uint32_t;

static const char* ERR_CORRUPT = "corrupt file.\n";

typedef struct {
    unsigned is_leave : 1;
    unsigned changed : 1;
    unsigned key_num : 10;
    unsigned page_num : 26; // begin with 1
    unsigned parent : 26;
} page_meta;

#define BODY_SIZE (PAGE_SIZE - (sizeof(page_meta)))

typedef struct {
    page_meta meta;
    char bytes[BODY_SIZE];
} page_t;

typedef struct {
    uint32_t key;
} key_t;

#define INTERNAL_KEYS_OFFSET sizeof(page_meta)
#define INTERNAL_KEYS_NUM ((BODY_SIZE - 4) / (sizeof(key_t) + sizeof(uint32_t)))
#define INTERNAL_POINTERS_OFFSET (INTERNAL_KEYS_OFFSET + (INTERNAL_KEYS_NUM * sizeof(key_t)))
#define INTERNAL_PADDING_SIZE (BODY_SIZE - INTERNAL_KEYS_NUM * (sizeof(key_t) + sizeof(uint32_t)) - sizeof(uint32_t))

typedef struct {
    page_meta meta;
    key_t keys[INTERNAL_KEYS_NUM];
    uint32_t pointers[INTERNAL_KEYS_NUM + 1];
    char padding[INTERNAL_PADDING_SIZE];
} internal_t;

typedef struct {
    int id;
    char username[32];
    char email[255];
} row_t;

#define LEAVE_KEYS_OFFSET 12
#define LEAVE_PAIR_NUM ((PAGE_SIZE - LEAVE_KEYS_OFFSET) / (sizeof(key_t) + sizeof(row_t)))
#define LEAVE_VALUE_OFFSET (LEAVE_KEYS_OFFSET + LEAVE_PAIR_NUM * sizeof(key_t))
#define LEAVE_PADDING_SIZE (PAGE_SIZE - LEAVE_VALUE_OFFSET - LEAVE_PAIR_NUM * sizeof(row_t))

typedef struct {
    page_meta meta;
    uint32_t next;
    key_t keys[LEAVE_PAIR_NUM];
    row_t rows[LEAVE_PAIR_NUM];
    char padding[LEAVE_PADDING_SIZE];
} leave_t;

typedef struct {
    int fd;
    uint32_t size;
    page_t* pages[MAX_PAGES];
    uint32_t page_num;
} pager_t;

#define TABLEMETA_PADDING_SIZE (PAGE_SIZE - sizeof(uint32_t))

typedef struct {
    uint32_t root_page_num;
    char padding[TABLEMETA_PADDING_SIZE];
} table_meta;

typedef struct {
    pager_t* pager;
    table_meta* meta; // page 0
} table_t;

pager_t* open_pager(char* filename);
void flush_pages(pager_t* pager);
table_t* open_db(char* filename);
void close_db(table_t* table);
int insert(table_t* table, key_t key, row_t value);
void print_table(table_t* table);