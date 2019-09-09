# WallPaper
使用xmake构建。https://github.com/xmake-io/xmake <br>

嵌入桌面参考：<br>
https://github.com/Francesco149/weebp<br>
https://github.com/giantapp-libraries/LiveWallpaperEngine<br>

构建工程：<br>
	xmake project -k vs2017<br>
直接编译：<br>
	xmake f -p windows --arch=x64<br>
	运行：<br>
	xmake run [exe 路径] [exe的参数]<br>

目标：<br>
1.hook桌面消息，将程序置于桌面图标下面,并能够响应鼠标 <br>
->初步完成，在win10 64位上测试通过.<br>
<br>
2.实现unity3d 桌面，使用video texture media特性<br>
->进行中<br>
