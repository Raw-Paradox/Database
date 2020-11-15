import ctypes
import pathlib
import random
import string
import fastapi
from lib import bplus
import pydantic
import uvicorn

database = bplus.Database('test.db')


class Body(pydantic.BaseModel):
    id: int
    username: str
    email: str


api = fastapi.FastAPI()


@api.post('/insert')
def insert(body: Body):
    database.insert(body.id, body.username, body.email)


uvicorn.run(api, host='0.0.0.0', port=8000)

database.close_db()

# if __name__ == "__main__":
# for i in range(0, 100):
#     id = random.randint(0, 100)
#     username = ''
#     for j in range(0, random.randint(0, 32)):
#         username += random.choice(string.ascii_letters)
#     email = ''
#     for j in range(0, random.randint(0, 255)):
#         email += random.choice(string.ascii_letters)
#     database.insert(id, username, email)
#     print(f'Insert {id} {username} {email}')
# database.print_table()
