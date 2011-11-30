in_tta.dll Ver3.2 (Beta12) 
=====================================================

*このプラグインは
本家(http://www.true-audio.com/)配布のwinamp用プラグインに
メディアライブラリ読み込み対応を付け加え、ID3v2周りを改変し
たものです。よって、Winamp Ver2.9以降に対応します。

動作確認はWinamp5.622で行いました。


*インストール方法
 WinampのフォルダにあるPluginsフォルダにin_tta.dllをコピー
 てください。また、Winampのベースフォルダにtag.dll
(https://github.com/downloads/bunbun042000/taglib-modified/tag.dll)をコピーしてください。

*更新履歴
2011-11-27 Beta12 不正なファイル名を渡された際にフリーズする問題を修正
                  アルバムアートなしのファイルを読み込むとフリーズすることがある問題を修正
2011-11-21 Beta11 ベースライブラリをlibtta++2.1に変更
                  taglibを同梱から外部dllを使う方法に変更
                  アルバムアート読み込み時のメモリリークを修正
2011-11-17 Beta10 taglibを1.7.0ベースに変更
                  コンパイラをVC2010に変更
2010-11-29 Beta9  アルバムアート読み込みに対応
2009-04-09 Beta8  コンパイラをVC2008に変更
                  ID3 Ver2.4においてYearタグをTDRL -> TDRCに変更
2007/10/23 Beta7  シーク位置ずれの修正
	                Format Conversionに対応
2006/07/03 Beta6  メディアライブラリにyear情報を付加するために小改正
2006/02/23 Beta5  Beta4での修正点が実際には修正されておらず,再修正
2005/12/19 Beta4  再生中に他のプログラムからttaファイルにアクセスできない問題点を修正
2005/12/16 Beta3  ID3v2 Ver2.3の読み込み不具合修正
	                (フレームサイズを正しくデコードしていない問題)
2005/12/14 Beta2

*配布
配布規定はLGPL2.1に従います。

*その他
この改変は私が勝手に行ったものであり、この改変版に関して
本家に問い合わせを行わないでください。

*謝辞
メディアライブラリ読み込み関連では
 おたちゃん さん作成のin_mpg123.dll 改悪版
のソースコードを参照しました。
  Web: http://www3.cypress.ne.jp/otachan/

ID3v2関連では
はせた さん作成のSTEP(SuperTagEditor改 Plugin Version (Nightmare))
のソースコードを参考にしました。
  Web: http://haseta2003.hp.infoseek.co.jp/

シーク関連の修正は
h0shu さん作成のパッチを参考にしました。
  Web: http://hoshustep.hp.infoseek.co.jp/dust.html

この場をお借りしてお礼申し上げます。

Yamagata Fumihiro
mailto:bunbun042000@gmail.com