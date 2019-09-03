kanzi-w64
=====

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/1385470b92804ac0a6854b71de5ee28d)](https://www.codacy.com/app/WhatTheBlock/kanzi-w64?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=WhatTheBlock/kanzi-w64&amp;utm_campaign=Badge_Grade)

此版本主要專注在修正Windows平台的編譯錯誤，並釋出x64執行檔<br>
未來也會找時間把help資訊中文化，方便新手使用<br><br>

kanzi為無損的檔案壓縮工具，保有高壓縮率的同時也大幅降低花費時間<br>
下方提供各壓縮工具的實測數據，歡迎比較。<br><br>

### [下載kanzi-w64](https://github.com/WhatTheBlock/kanzi-w64/releases) <br><br>

演算法等詳細說明請看原作者的wiki → https://github.com/flanglet/kanzi/wiki <br><br>

測試數據
-------
#### 測試平台：
i7-6700HQ @2.60GHz, 20GB RAM, Windows 10 Pro 1903 (x64)

#### 測試的壓縮工具：
1. WinRAR v5.71.0
2. PACL64 v9.00b build 20190606 (pa format)
3. UHARC v0.6a
4. 7-Zip v19.00
5. BCM v1.30
6. Kanzi v1.6 build 20190903c
7. Paq8px v178
8. Paq8pxd v63
9. cmix v18 <br><br>

### 1. enwik8 benchmark
|        Compressor             | Encoding (sec) | Decoding (sec) | Size (Byte)  | compare to zip |
|-------------------------------|----------------|----------------|--------------|----------------|
|Original     	                |                |                |100,000,000   |	          |
|winrar -afzip -m5              |2 	         |1               |36,213,592    |100%            |
|rar -md64 -m5                  |23 	         |1               |27,644,573    |76.3%           |
|pacomp -c19                    |21.84 	         |4.12            |26,494,134    |73.1%           |
|uharc -mx -md32768             |94 	         |74.2            |23,911,123    |66.0%           |
|7z -m0=ppmd:o32:mem1024m -mx9  |41.06           |42.78           |21,918,001    |60.5%           |
|bcm -b100m                     |23.88           |31.45           |20,789,664    |57.4%           |
|**kanzi -l 8 -b 100m**         |**50.57**       |**52.11**       |**19,142,982**|52.8%           |
|paq8px -9eta                   |20462.48        |21376.88        |16,457,780    |45.4%           |
|paq8pxd -s15                   |?	         |?               |15,967,201    |44.1%           |
|cmix                           |?               |?               |14,838,332    |40.9%           |

### 2. Silesia corpus benchmark
|        Compressor             | Encoding (sec) | Decoding (sec) | Size (Byte)  | compare to zip |
|-------------------------------|----------------|----------------|--------------|----------------|
|Original     	                |                |                |211,938,580   |	          |
|winrar -afzip -m5              |- 	         |-               |67,373,465    |100%            |
|rar -md256 -m5 -qo-            |15.24 	         |1.57            |53,451,158    |79.3%           |
|pacomp -c19 -mt                |43.13 	         |8.67            |48,109,141    |71.4%           |
|7z -m0=ppmd:o32:mem1024m -mx9  |50.42           |54.49           |47,850,118    |71.0%           |
|bcm -b100m                     |-               |-               |46,506,680    |69.0%           |
|uharc -mx -md32768             |124.2	         |109.2           |45,172,307    |67.0%           |
|**kanzi -l 8 -b 100m -j 6**    |**56.19**       |**56.49**       |**40,477,451**|60.0%           |
|cmix                           |?               |?               |28,437,634    |42.4%           |

<br>

#### 備註：
- 所有數據皆以該壓縮工具的最佳壓縮率為主，第一筆數據例外，因Zip將作為基準點
- 順序以壓縮後檔案由大到小排列
- "?"表示尚未以自己的電腦測試
- 下載[enwik8測試檔](http://mattmahoney.net/dc/enwik8.zip)
- 下載[Silesia測試檔](http://mattmahoney.net/dc/silesia.zip)

<br>

使用方式
-------
#### 壓縮指令

<pre><code>   -v, --verbose=<level>
        0=silent, 1=default, 2=display details, 3=display configuration,
        4=display block size and timings, 5=display extra information
        Verbosity is reduced to 1 when files are processed concurrently
        Verbosity is silently reduced to 0 when the output is 'stdout'
        (EX: The source is a directory and the number of jobs > 1).


   -f, --force
        overwrite the output file if it already exists


   -i, --input=<inputName>
        mandatory name of the input file or directory or 'stdin'
        When the source is a directory, all files in it will be processed.
        Provide \. at the end of the directory name to avoid recursion
        (EX: myDir\. => no recursion)


   -o, --output=<outputName>
        optional name of the output file or directory (defaults to
        <inputName.knz>) or 'none' or 'stdout'. 'stdout' is not valid
        when the number of jobs is greater than 1.

   -b, --block=<size>
        size of blocks, multiple of 16 (default 1 MB, max 1 GB, min 1 KB).


   -l, --level=<compression>
        set the compression level [0..8]
        Providing this option forces entropy and transform.
        0=None&None (store), 1=TEXT+LZ&HUFFMAN, 2=TEXT+ROLZ
        3=TEXT+ROLZX, 4=TEXT+BWT+RANK+ZRLT&ANS0, 5=TEXT+BWT+SRT+ZRLT&FPAQ
        6=BWT&CM, 7=X86+RLT+TEXT&TPAQ, 8=X86+RLT+TEXT&TPAQX


   -e, --entropy=<codec>
        entropy codec [None|Huffman|ANS0|ANS1|Range|FPAQ|TPAQ|TPAQX|CM]
        (default is ANS0)


   -t, --transform=<codec>
        transform [None|BWT|BWTS|LZ|ROLZ|ROLZX|RLT|ZRLT|MTFT]
                  [RANK|SRT|TEXT|X86]
        EX: BWT+RANK or BWTS+MTFT (default is BWT+RANK+ZRLT)


   -x, --checksum
        enable block checksum


   -s, --skip
        copy blocks with high entropy instead of compressing them.

   -j, --jobs=<jobs>
        maximum number of jobs the program may start concurrently
        (default is 1, maximum is 64).


EX. kanzi -c -i foo.txt -o none -b 4m -l 4 -v 3

EX. kanzi -c -i foo.txt -f -t BWT+MTFT+ZRLT -b 4m -e FPAQ -v 3 -j 4

EX. kanzi --compress --input=foo.txt --output=foo.knz --force
          --transform=BWT+MTFT+ZRLT --block=4m --entropy=FPAQ --verbose=3 --jobs=4
</code></pre>

#### 解壓縮指令

<pre><code>   -v, --verbose=<level>
        0=silent, 1=default, 2=display details, 3=display configuration,
        4=display block size and timings, 5=display extra information
        Verbosity is reduced to 1 when files are processed concurrently
        Verbosity is silently reduced to 0 when the output is 'stdout'
        (EX: The source is a directory and the number of jobs > 1).


   -f, --force
        overwrite the output file if it already exists


   -i, --input=<inputName>
        mandatory name of the input file or directory or 'stdin'
        When the source is a directory, all files in it will be processed.
        Provide \. at the end of the directory name to avoid recursion
        (EX: myDir\. => no recursion)


   -o, --output=<outputName>
        optional name of the output file or directory (defaults to
        <inputName.knz>) or 'none' or 'stdout'. 'stdout' is not valid
        when the number of jobs is greater than 1.

   -j, --jobs=<jobs>
        maximum number of jobs the program may start concurrently
        (default is 1, maximum is 64).


EX. kanzi -d -i foo.knz -f -v 2 -j 2

EX. kanzi --decompress --input=foo.knz --force --verbose=2 --jobs=2
</code></pre>
