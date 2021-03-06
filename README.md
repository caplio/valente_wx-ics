<PRE>
■カーネルビルドのメモ

このカーネルをコンパイルする為のメモです

最初にクロスコンパイラへのPATHを通しておきます。

例）
$ cat .bashrc 
PATH=~/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin:$PATH

make valente_wx_defconfig で config を makeします。
その後、変更したければmake menuconfigなどで弄って下さい。

設定が終わったら、makeでbuildします。
エラーなく最後まで走れば、arch/arm/boot/zImage が作成されます。

これを、ramdiskとくっつければ完成です。

例）
$ find -name '*.ko' -exec cp -av {} ./boot.img-ramdisk/system/lib/module \;
$ repack-bootimg.pl arch/arm/boot/zImage ./boot.img-ramdisk ./output/boot.img

zImageからカーネルイメージ作成までの部分の shellスクリプトがあります。

create-bootimage.sh  を実行すれば、output配下にbootxxxx-xx.img として、
カーネルイメージが出力されますのでご利用下さい。
($ fastboot boot bootxxxx-xx.img で起動するイメージファイルです)


configなどを弄ったらmake cleanしてからmakeし直しが良いでしょう。
https://pub.slateblue.tk/memo/day2297.html　にも少し書いたので、
こちらも見てみて下さい。

----------------------------------------------
[make memo]

$ make valente_wx_defconfig
$ make 

cp "*.ko" to <boot.img-ramdisk>/system/lib/module/
cp arch/arm/boot/zImage ./

run repack-bootimg.pl
"repack-bootimg.pl zImage <boot.img-ramdisk> <output file>"

 ex)
 $ repack-bootimg.pl zImage boot.img-ramdisk boot-new.img

test boot

 $ fastboot boot boot-new.img
 
</PRE>
