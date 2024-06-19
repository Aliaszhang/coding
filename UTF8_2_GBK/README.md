## 本文说明如何把大数组转换成bin文件方便存放到Flash上
> 背景： 需要把UTF-8转换为GBK编码。
> 因为GBK和UTF-8没有啥关系，但是Unicode和UTF-8是可以相互转化的，网上查了资料后，发现Fatfs提供了Unicode到GBK的转换。所以可以通过UTF-8转Unicode转GBK来实现
> 但是Fatfs提供的unicode与GBK的转换是通过查表来实现的，而且这个表格171k，太大了，放在代码中不方便做远程升级。于是参考正点原子提供的文档，可以把数组转换成bin文件单独放在flash中使用，这样就解决问题了

___

### 1. 准备好文件

- 找到FATFS提供的源文件：`cc936.c` 或者`ffunicode.c`, 两个文件都可以后者是all in one，我使用的是`cc936.c`
- 把数组分别拷贝到两个txt文件中,我取名为：uni2oem.txt 和 oem2uni.txt (放一起也行，代码区分一下)

### 2. 打开软件`C2B转换助手 V2.1.exe`

- 默认配置，不用改（默认小端存储，改成大端存储后转换函数失效，可能需要对转换函数也进行修改）
- 分别打开上一步准备的两个txt文件
- 点击转换，会在当前目录中生成同名的bin文件

### 3. 把bin文件存放到flash中

- 使用J-flash创建工程
- 把生成的字库bin文件拷贝到指定的位置，比如：
```
#define UNI2OEM_ADDRESS	0x08200000
#define UNI2OEM_SIZE	87172
#define OEM2UNI_ADDRESS 0x08216000
#define OEM2UNI_SIZE    UNI2OEM_SIZE
```

### 4. 修改代码

- ff_convert修改后如下：
```
WCHAR ff_convert (	/* Converted code, 0 means conversion error */
	WCHAR	chr,	/* Character code to be converted */
	UINT	dir		/* 0: Unicode to OEM code, 1: OEM code to Unicode */
)
{
	const WCHAR *p;
	WCHAR c;
	int i, n, li, hi;


	if (chr < 0x80) {	/* ASCII */
		c = chr;
	} else {
		if (dir) {		/* OEM code to unicode */
			p = (WCHAR *)OEM2UNI_ADDRESS;       // 替换成Flash地址
			hi = OEM2UNI_SIZE / 4 - 1;          // 直接给出数组大小(文件大小)
		} else {		/* Unicode to OEM code */
			p = (WCHAR *)UNI2OEM_ADDRESS;       // 替换成FLAHS地址
			hi = UNI2OEM_SIZE / 4 - 1;          // 直接给出数组大小(文件大小)
		}
		li = 0;
		for (n = 16; n; n--) {
			i = li + (hi - li) / 2;
			if (chr == p[i * 2]) break;
			if (chr > p[i * 2])
				li = i;
			else
				hi = i;
		}
		c = n ? p[i * 2 + 1] : 0;
	}

	return c;
}
```

- 添加unicode和UTF-8相互转换的函数
```
/**
 * @brief 将一个字符的 Unicode 编码转换成 UTF-8 编码. 安全起见，**utf8缓存必须大于等于6**
 * 
 * @param unicode Unicode编码
 * @param utf8 UTF8 缓存指针，内存至少6个空间
 * @return int 0-失败，其他-utf8编码长度
 */
int unicode_2_utf8(WCHAR unicode, uint8_t *utf8) 
{
	int len = 0;
    if (unicode <= 0x7F) { 			// 0 - 7F: 0xxxxxxx
        utf8[0] = (uint8_t)(unicode & 0x7F);
        len = 1;
    } else if (unicode <= 0x7FF) { // 80 - 7FF: 110XXXXX 10XXXXXX
        utf8[0] = (uint8_t)(0xC0 | ((unicode >> 6) & 0x1F));
        utf8[1] = (uint8_t)(0x80 | (unicode & 0x3F));
        len = 2;
    } else if (unicode <= 0xFFFF) { //800 - FFFF: 1110xxxx 10xxxxxx 10xxxxxx
        utf8[0] = (uint8_t)(0xE0 | ((unicode >> 12) & 0x0F));
        utf8[1] = (uint8_t)(0x80 | ((unicode >> 6) & 0x3F));
        utf8[2] = (uint8_t)(0x80 | (unicode & 0x3F));
        len = 3;
    } else if (unicode <= 0x1FFFFF) { // 10000 - 1FFFFF : 11110XXX 10XXXXXX 10XXXXXX 10XXXXXX
        utf8[0] = (uint8_t)(0xF0 | ((unicode >> 18) & 0x07));
        utf8[1] = (uint8_t)(0x80 | ((unicode >> 12) & 0x3F));
        utf8[2] = (uint8_t)(0x80 | ((unicode >> 6) & 0x3F));
		utf8[3] = (uint8_t)(0x80 | (unicode & 0x3F));
		len = 4;
	} else if (unicode <= 0x3FFFFFF) { // 200000 - 3FFFFFF : 111110XX 10xxxxxx 10XXXXXX 10XXXXXX 10XXXXXX
        utf8[0] = (uint8_t)(0xF8 | ((unicode >> 24) & 0x03));
		utf8[1] = (uint8_t)(0x80 | ((unicode >> 18) & 0x3F));
        utf8[2] = (uint8_t)(0x80 | ((unicode >> 12) & 0x3F));
        utf8[3] = (uint8_t)(0x80 | ((unicode >> 6) & 0x3F));
		utf8[4] = (uint8_t)(0x80 | (unicode & 0x3F));
		len = 5;
	} else if (unicode <= 0x7FFFFFFF) { // 4000000 - 7FFFFFFF : 1111110X 10xxxxxx 10xxxxxx 10XXXXXX 10XXXXXX 10XXXXXX
        utf8[0] = (uint8_t)(0xFC | ((unicode >> 30) & 0x01));
		utf8[1] = (uint8_t)(0x80 | ((unicode >> 24) & 0x3F));
		utf8[2] = (uint8_t)(0x80 | ((unicode >> 18) & 0x3F));
        utf8[3] = (uint8_t)(0x80 | ((unicode >> 12) & 0x3F));
        utf8[4] = (uint8_t)(0x80 | ((unicode >> 6) & 0x3F));
		utf8[5] = (uint8_t)(0x80 | (unicode & 0x3F));
		len = 6;
	}

	return len;
}

/**
 * @brief 将一个 UTF-8 编码转换成 Unicode 编码并返回UTF-8的长度
 * 
 * @param utf8 待转换的utf8编码buffer指针
 * @param unic 转换后的Unicode编码的指针
 * @return WCHAR 成功消耗的utf-8的长度，失败0
 */
int utf8_2_unicode(uint8_t *utf8, WCHAR *unic) 
{
    WCHAR unicode = 0;
	int len = 0;

	if (*utf8 < 0x80) { 
		// 0xxxxxxx					
		unicode = *utf8; 
		len = 1;
	} else if ((*utf8 & 0xE0) == 0xC0 && (*(utf8 + 1) & 0xC0) == 0x80) { 	
		// 110xxxxx 10xxxxxx
		unicode = (((*utf8 & 0x1F) << 6) | (*(utf8+1) & 0x3F));
		len = 2;
	} else if ((*utf8 & 0xF0) == 0xE0 && (*(utf8 + 1) & 0xC0) == 0x80 && (*(utf8 + 2) & 0xC0) == 0x80) {
		// 1110xxxx 10xxxxxx 10xxxxxx
		unicode = (((*utf8 & 0x0F) << 12) | ((*(utf8+1) & 0x3F) << 6) | (*(utf8+2) & 0x3F));
		len = 3;
	} else if ((*utf8 & 0xF8) == 0xF0 && (*(utf8 + 1) & 0xC0) == 0x80 &&\
				(*(utf8 + 2) & 0xC0) == 0x80 && (*(utf8 + 3) & 0xC0) == 0x80) {
		// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		unicode = (((*utf8 & 0x07) << 18) | ((*(utf8+1) & 0x3F) << 12) | ((*(utf8+2) & 0x3F) << 6) | (*(utf8+3) & 0x3F));
		len = 4;
	} else if (((*utf8 & 0xFC) == 0xF8) && ((*(utf8+1) & 0xC0) == 0x80) &&\
		((*(utf8 + 2) & 0xC0) ==0x80) && ((*(utf8+3) & 0xC0) == 0x80) && ((*(utf8+4) & 0xC0) == 0x80)) {
		// 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		unicode = (((*utf8 & 0x03) << 24) | ((*(utf8+1) & 0x3F) << 18) |\
					((*(utf8+2) & 0x3F) << 12) | ((*(utf8+3) & 0x3F) << 6) | ((*(utf8+4) & 0x3F)));
		len = 5;
	} else if (((*utf8 & 0xFE) == 0xFC) && ((*(utf8+1) & 0xC0) == 0x80) && ((*(utf8+2) & 0xC0) == 0x80) &&\
		((*(utf8+3) & 0xC0) == 0x80) && ((*(utf8+4) & 0xC0) == 0x80) && ((*(utf8+5) & 0xC0) == 0x80)) {
		// 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		unicode = (((*utf8 & 0x01) << 30) | ((*(utf8+1) & 0x3F) << 24) | ((*(utf8+2) & 0x3F) << 18) |\
					((*(utf8+3) * 0x3F) << 12) | ((*(utf8+4) & 0x3F) << 6) | ((*(utf8+5) & 0x3F)));
		len = 6;
	}

	*unic = unicode;
    return len;
}
```
- 封装出UTF-8数组直接转换成GBK数组的的接口
```
/**
 * @brief 把传入的utf8编码的数组转换为gbk数组，并返回gbk数组的长度
 * 
 * @param utf8 utf8 编码的数组指针
 * @param utf8_len utf8 编码的数组长度
 * @param gbk 存放gbk编码的数组指针
 * @return int 成功返回gbk编码的长度，失败返回-1
 */
int utf8_2_gbk(uint8_t *utf8, uint16_t utf8_len, uint16_t *gbk)
{
	int len = 0, gbk_len = 0;
	int count = 0;
	uint8_t *pin = utf8;
	uint16_t *pout = gbk;
	WCHAR unic = 0;
	WCHAR one_gbk = 0;

	if (utf8 == NULL || gbk == NULL || utf8_len == 0)
		return -1;

	while (count < utf8_len) {
		len = utf8_2_unicode(pin + count, &unic);
		if (len > 0) {
			one_gbk = ff_convert(unic, 0);	
			if (one_gbk > 0) {
				*(pout + gbk_len) = one_gbk;
				gbk_len++;
			} 
			count += len;
		} else {
			return -1;
		}
	}

	return gbk_len;
}
```