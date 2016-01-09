fusermount -u fstest
rm fs_iso
touch fs_iso
bash build.sh
./fs fstest -f
