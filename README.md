# LanDetection_traditional_image_process
传统图像处理方法，车道线检测


> 分层投影法车道线检测，起初是本科的项目，后在某公司有过实际应用。算是第一次独立完成较大型项目，所以接口设计得很差，参数（相机标定参数，行道线宽度等）管理很糟糕，
对一个特定的数据集需要重新调整很多参数（主要是相机的标定参数相关，对于环境实际上很鲁棒）  
> 当初没有工程经验和工程思维，只想着迅速做出功能和在数据集上的效果，所以对外接口不友好，以后有精力会修改。

一些测试样例：
![测试样例](https://github.com/Kxy-Moriaty/LanDetection_traditional_image_process/blob/master/Image/%E6%B5%8B%E8%AF%95.png)


分块投影算法是我的导师任明武教授提出的，我只是进行了实现。
具体实现原理请见 [实现原理](https://github.com/Kxy-Moriaty/LanDetection_traditional_image_process/blob/master/%E5%AE%9E%E7%8E%B0%E5%8E%9F%E7%90%86.docx)
