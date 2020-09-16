rm -rf opk
mkdir -p opk
cp ../build/ababisme opk
cp data/default.gcw0.desktop opk
cp data/icon.png opk
mksquashfs opk ababisme.opk -all-root -noappend -no-exports -no-xattrs -no-progress > /dev/null
# rm -rf opk
