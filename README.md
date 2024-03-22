# utils
 many utils
 
Usage:
```
utils <function> [option...]
 where function is:
 help - show this help
 
 help <function> - show function help

 delgaaps <folder> <file_list>
    Where:
    <folder> - path to the folder where the image was unpacked
    <file_list> - a file with a list to remove gapps and more
    For example:
    delgaaps "/sdcard/Unpacker" "/sdcard/gappslist.txt"

 sdat2img <transfer_list> <new_dat_file> <output_img_file>
    Where <output_img_file> path to output img file, optionaly

writekey <image_file> <offset> <-fhb> <file_key_value>
    Where:
        -b backup
        -f file key
        -h hex value

cut <file> <offset> <offset or length> <cute_dir>

copy <file> <offset> <offset or length> <copy_dir>

insert <file> <offset> <insert_file>

foffset <file> <hexstring> [option...]
    where option is:
        -s <offset> - start offset (DEC or 0xHEX), negative value for offset from end of file
        -l <length> - search region length (DEC or 0xHEX)
        -n <num> - maximum number of offsets (DEC or 0xHEX)
        -b <size> - size of buffer in KB (DEC or 0xHEX), default is 1024
        -r - reverse search
        -d - display offsets in decimal (default is hex)
        -hu - display offsets in uppercase hex (only if -d option is not specified)
        -hp - display offset 0x prefix for hex (only if -d option is not specified)
        -hup - combination of -hu and -hp options
        -o - display offsets in one string separated by space
        -q - no statistic message
        hexstring digit pairs that can be separated using:
            space, period(.), comma(,), dash(-) or colon(:), e.g. 41.35:AA.55

hexpatch <file> <what_find in hex> <what_replace in hex> <way>
    Where way as integer:
    way = 0 - first find from begin file, by default
    way = 1 - all finds in file
    way = -1 - reverse first find, from end file

kerver <kernel file>
    kernel file may be compressed to gzip
    
fstab_fix <directory1> <directory2> ...
    fix RO to RW in fstab files

shared_block_detector <file>
    detect shared_blocks in sparse ext4 and raw ext4 files

file_explorer [directory] [options] [filters]
    Explore files and directories in the specified directory."
                 Options:
                 -d    Show only directories, ignoring filters.
                 -f    Show only files, applying filters.
                 Filters:
                 By default, the following filters are applied to files: .img, .dat, .br, .list.
                 Additional filters can be specified after the directory path.
                 "Example: file_explorer /path/to/directory .txt .pdf
```