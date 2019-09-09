# WallPaper
使用xmake构建。https://github.com/xmake-io/xmake

嵌入桌面参考：
https://github.com/Francesco149/weebp
https://github.com/giantapp-libraries/LiveWallpaperEngine

构建工程：
	xmake project -k vs2017
直接编译：
	xmake f -p windows --arch=x64
	运行：
	xmake run [exe 路径] [exe的参数]

目标：
1.hook桌面消息，将程序置于桌面图标下面,并能够响应鼠标 
->初步完成，在win10 64位上测试通过.

2.实现unity3d 桌面，使用video texture media特性
->进行中
