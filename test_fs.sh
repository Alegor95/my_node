GREEN='\033[0;32m'
EMPTY='\033[0m'
echo -e "${GREEN}------------CREATE   TESTS-------------${EMPTY}"
mkdir fstest/a
ls fstest
touch fstest/a/b.txt
ls fstest/a
echo -e "${GREEN}------------CREATE    DONE-------------${EMPTY}"
echo -e "${GREEN}------------CONTENT  TESTS-------------${EMPTY}"
echo "test content for file. it must be enough long for use
inderect blocks of inode structure">fstest/a/c.txt
ls fstest/a
cat fstest/a/c.txt
echo "another test string">fstest/a/b.txt
cat fstest/a/b.txt
echo -e "${GREEN}------------CONTENT   DONE------------${EMPTY}"
echo -e "${GREEN}------------REMOVE   TESTS------------${EMPTY}"
rmdir fstest/a
rm fstest/a/c.txt
ls fstest/a
rm fstest/a/b.txt
ls fstest/a
rmdir fstest/a
ls fstest
echo -e "${GREEN}-----------REMOVE     DONE--------------${EMPTY}"
echo -e "${GREEN}-----------TESTS      DONE--------------${EMPTY}"
