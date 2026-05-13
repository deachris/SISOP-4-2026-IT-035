FROM ubuntu:latest

RUN apt-get update && apt-get install -y gcc pkg-config fuse3 libfuse3-dev

WORKDIR /app

COPY . .

RUN gcc fuse.c -o fuse `pkg-config fuse3 --cflags --libs`

EXPOSE 9000

CMD ["./server"]
