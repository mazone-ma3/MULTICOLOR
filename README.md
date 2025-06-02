# MULTICOLOR
多色表示実験 読み物 By m@3

・まずmagick等で24bitbitmapに縮小変換します。.batをつけておきます。

画像比率は縮小前に前もって合わせておきます。

・X1turboZ

320x200ドット4096色bitmapからconv24.cでrawデータを生成後、div.cで分割。

BLOADX1Z.cをz88dkでコンパイルしてLSX-Dodgers上で実行。

・FM77AV

X1turboZと同じようにrawデータを分割生成してfm7conv.cで変換。(fmv.bat付属)

mmrg.cをGCC6809でコンパイルしてF-BASICからfmld.basを実行。

・PC-88VA

VABMP.cをWatcom Cでコンパイルして640x200ドット65536色bitmapを直接読みます。
