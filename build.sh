GREEN='\033[0;32m'
EMPTY='\033[0m'
gcc -c main.c -o main.o `pkg-config fuse --cflags --libs`
gcc -c mynode.c -o mynode.o
gcc -c file_service.c -o file_service.o
gcc -Wall main.o mynode.o file_service.o `pkg-config fuse --cflags --libs` -o fs
rm main.o mynode.o file_service.o
echo -e "${GREEN}--------------BUILD DONE--------------${EMPTY}"
