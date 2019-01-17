#!/bin/bash

tar_function(){
    tar cf - $1 | dd of=~/Congestion_Control_with_Bit_torrent/example/$2.tar bs=512K conv=sync count=2
}
whole_dic=~/Congestion_Control_with_Bit_torrent
dictionary=/home/lzx/Congestion_Control_with_Bit_torrent/example
chunk=512
fix_size=512K
file_size_kb=
chunksize=0
fn_b=
extension=
for filename in $dictionary/*;do
    fn=$(basename "$filename")
    fn_b="${fn%.*}"
    extension="${fn##*.}"
    if [ "$extension" = "gif" ];then
        file_size_kb=$(du -k "$filename" | cut -f1)
        echo "the file size of $fn is $file_size_kb"
        chunksize=$((file_size_kb / 512))
        echo "this file has $chunksize chunks"
        case $chunksize in
            0 ) $(tar_function $fn $fn_b)
                ;;
            1 ) $(tar_function $fn $fn_b)
                ;;
            2 ) $(tar_function $fn $fn_b)
                ;;
            * ) $(tar_function $fn $fn_b)
        esac
    fi
done
cat ~/Congestion_Control_with_Bit_torrent/example/*.tar > all.tar

for filename in $dictionary/*;do
    fn=$(basename "$filename")
    fn_b="${fn%.*}"
    extension="${fn##*.}"
    if [ "$extension" = "tar" ];then
        $whole_dic/make-chunks $fn > $fn_b.chunks
    fi
done

sed -i '1s/^/Chunks:\n/' all.chunks
sed -i '1s/^/File: \/home\/lzx\/Congestion_Control_with_Bit_torrent\/example\/all.tar\n/' all.chunks

mv $dictionary/all.chunks $dictionary/all.masterchunks



