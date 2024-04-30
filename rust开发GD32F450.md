# 1. 生成pac库

- 找到`GD32F4xx.svd`，我本地有keil，开发过对应的C程序，直接找到pack安装路径`C:\Users\xxxxxxxx\AppData\Local\Arm\Packs\GigaDevice\GD32F4xx_DFP\3.0.3\SVD\GD32F4xx.svd`
- 创建库工程：`cargo new --lib gd32f4xx-pac`
- 进入工程，删除src：`rm -rf src/`
- 把`GD32F4xx.svd`拷贝到工程根目录
- 执行命令：` svd2rust -i GD32F4xx.svd --target=cortex-m`
- 报错：
```
[INFO  svd2rust] Parsing device from SVD file
[ERROR svd2rust] Error parsing SVD XML file

    Caused by:
        unexpected XML declaration at 1:3
```

- 用STM32的尝试，一次就通过,证明svd2rust没有问题，那就一定是GD32F4XX.SVD的问题了！
```
$ svd2rust -i STM32F429.svd --target=cortex-m
[INFO  svd2rust] Parsing device from SVD file
[INFO  svd2rust] Rendering device
```

- 对比ST和GD，发现开头多了两个空格，删除后报错变成了
```
[INFO  svd2rust] Parsing device from SVD file
[ERROR svd2rust] Error parsing SVD XML file

    Caused by:
        0: In device `GD32F4xx`
        1: In peripheral `RCU`
        2: In register `ADDINT`
        3: In field `IRC48MSTBIC`
        4: Parsing unknown access at 33003:15
        5: unknown access variant 'write' found
```

网上查找原因看看有人遇到过同样问题没。还真有！外国友人回复说GD32F4XX.SVD不是按照官方说明写的，更改有问题的地方
```
$ git diff GD32F4xx.svd
diff --git a/GD32F4xx.svd b/GD32F4xx.svd
index 8b99790..356a002 100644
--- a/GD32F4xx.svd
+++ b/GD32F4xx.svd
@@ -1,4 +1,4 @@
-  <?xml version="1.0" encoding="utf-8" standalone="no"?>
+<?xml version="1.0" encoding="utf-8" standalone="no"?>
 <device schemaVersion="1.1"
 xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
 xs:noNamespaceSchemaLocation="CMSIS-SVD_Schema_1_1.xsd">
@@ -33001,7 +33001,7 @@ frequency</description>
:...skipping...
diff --git a/GD32F4xx.svd b/GD32F4xx.svd
index 8b99790..356a002 100644
--- a/GD32F4xx.svd
+++ b/GD32F4xx.svd
@@ -1,4 +1,4 @@
-  <?xml version="1.0" encoding="utf-8" standalone="no"?>
+<?xml version="1.0" encoding="utf-8" standalone="no"?>
 <device schemaVersion="1.1"
 xmlns:xs="http://www.w3.org/2001/XMLSchema-instance"
 xs:noNamespaceSchemaLocation="CMSIS-SVD_Schema_1_1.xsd">
@@ -33001,7 +33001,7 @@ frequency</description>
               <description>Internal 48 MHz RC oscillator Stabilization Interrupt Clear</description>
               <bitOffset>22</bitOffset>
               <bitWidth>1</bitWidth>
-              <access>write</access>
+              <access>write-only</access>
             </field>
           </fields>
         </register>
@@ -33092,7 +33092,7 @@ frequency</description>
               <description>The key of RCU_DSV registe</description>
               <bitOffset>0</bitOffset>
               <bitWidth>32</bitWidth>
-              <access>write</access>
+              <access>write-only</access>
             </field>
           </fields>
         </register>
@@ -33126,7 +33126,7 @@ frequency</description>
         <usage>registers</usage>
       </addressBlock>
       <interrupt>
-        <name>RTC_T    amper</name>
+        <name>RTC_Tamper</name>
         <value>2</value>
       </interrupt>
       <interrupt>
@@ -39757,7 +39757,7 @@ frequency</description>
           <description>Current pixel position register</description>
           <addressOffset>0x44</addressOffset>
           <size>0x20</size>
-          <access>read</access>
+          <access>read-only</access>
           <resetValue>0x00000000</resetValue>
           <fields>
             <field>
@@ -40994,14 +40994,14 @@ frequency</description>
               <description>End of block flag</description>
               <bitOffset>12</bitOffset>
               <bitWidth>1</bitWidth>
-              <access>write</access>
+              <access>write-only</access>
             </field>
             <field>
               <name>RTF</name>
               <description>Receiver timeout flag</description>
               <bitOffset>11</bitOffset>
               <bitWidth>1</bitWidth>
-               <access>write</access>
+               <access>write-only</access>
             </field>
            </fields>
         </register>
(END)
```

- 再次执行命令：`svd2rust -i GD32F4xx.svd --target=cortex-m`,编译通过

```
$ svd2rust -i GD32F4xx.svd --target=cortex-m
[INFO  svd2rust] Parsing device from SVD file
[INFO  svd2rust] Rendering device
```

- 执行命令：`form -i lib.rs -o src/`，会在src目录下生成代码
- 删除lib.rs
- 在工程的cargo.toml中指定库的依赖和路径：`gd32f4xx-pac = { path = "./gd32f4xx-pac"}`

  # 2. 新建工程和编写代码

  Todo
