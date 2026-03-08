#ifndef __FIFO_H__
#define __FIFO_H__

#ifdef __cplusplus
extern "C" {
#endif
    #include "stdint.h"
    /*
    /环形缓冲区实现FIFO
    /两个指针：head与tail，初始化时两者都为0
    /写入队列时先写入head指向的地址，再将head向前移动
    /读取队列时先增加tail的值，再读取tail指向的地址，之后该地址作废
    /因此，FIFO中有效数据处于tail之前，head之后（不包括指针指向本身）
    /FIFO容量为数组大小-1
    */
    //长度最大255的FIFO使用该结构体
    struct FIFO_byte_t {
        uint8_t length;//FIFO长度
        uint8_t head;//FIFO头指针
        uint8_t tail;//FIFO尾指针
        uint8_t *content;//FIFO内容指针
    };
    
    //长度最大65535的FIFO使用该结构体
    struct FIFO_word_t {
        uint16_t length;//FIFO长度
        uint16_t head;//FIFO头指针
        uint16_t tail;//FIFO尾指针
        uint8_t *content;//FIFO内容指针
    };
    
    //长度最大65535的FIFO使用该结构体
    struct FIFO_dword_t {
        uint32_t length;//FIFO长度
        uint32_t head;//FIFO头指针
        uint32_t tail;//FIFO尾指针
        uint8_t *content;//FIFO内容指针
    };

    //FIFO定义宏
    #define FIFO_Create(type, name, length) static uint8_t name##_content[length + 1];\
                                            struct type name = {length, 0, length, name##_content}

/*******************************8位长度FIFO系列********************************/
//返回FIFO内元素个数
uint8_t GetFIFOLength_Byte(struct FIFO_byte_t *p);
//向FIFO内填充一个数据，位置为head
void FIFO_In_Byte(struct FIFO_byte_t *p, uint8_t data);
//从FIFO中取出一个数据，位置为tail+1
uint8_t FIFO_Out_Byte(struct FIFO_byte_t *p);
//从FIFO头部向后读取指定偏移量的数据
uint8_t FIFO_QueryfromHead_Byte(struct FIFO_byte_t *p, uint8_t offset);
//从FIFO尾部向前读取指定偏移量的数据
uint8_t FIFO_QueryfromTail_Byte(struct FIFO_byte_t *p, uint8_t offset);
//从FIFO头部向后读取指定偏移量的数据，不检查长度
uint8_t FIFO_QueryfromHead_Byte_NoCheck(struct FIFO_byte_t *p, uint8_t offset);
//从FIFO尾部向前读取指定偏移量的数据，不检查长度
uint8_t FIFO_QueryfromTail_Byte_NoCheck(struct FIFO_byte_t *p, uint8_t offset);
//从前向后删除FIFO内容，不取出数据，通过向后移动头指针实现
void FIFO_DeletefromHead_Byte(struct FIFO_byte_t *p, uint8_t num);
//从后向前删除FIFO内容，不取出数据，通过向前移动尾指针实现
void FIFO_DeletefromTail_Byte(struct FIFO_byte_t *p, uint8_t num);


/*******************************16位长度FIFO系列********************************/
//返回FIFO内元素个数
uint16_t GetFIFOLength_Word(struct FIFO_word_t *p);
//向FIFO内填充一个数据，位置为head
void FIFO_In_Word(struct FIFO_word_t *p, uint8_t data);
//从FIFO中取出一个数据，位置为tail+1
uint8_t FIFO_Out_Word(struct FIFO_word_t *p);
//从FIFO头部向后读取指定偏移量的数据
uint8_t FIFO_QueryfromHead_Word(struct FIFO_word_t *p, uint16_t offset);
//从FIFO尾部向前读取指定偏移量的数据
uint8_t FIFO_QueryfromTail_Word(struct FIFO_word_t *p, uint16_t offset);
//从FIFO头部向后读取指定偏移量的数据，不检查长度
uint8_t FIFO_QueryfromHead_Word_NoCheck(struct FIFO_word_t *p, uint16_t offset);
//从FIFO尾部向前读取指定偏移量的数据，不检查长度
uint8_t FIFO_QueryfromTail_Word_NoCheck(struct FIFO_word_t *p, uint16_t offset);
//从前向后删除FIFO内容，不取出数据，通过向后移动头指针实现
void FIFO_DeletefromHead_Word(struct FIFO_word_t *p, uint16_t num);
//从后向前删除FIFO内容，不取出数据，通过向前移动尾指针实现
void FIFO_DeletefromTail_Word(struct FIFO_word_t *p, uint16_t num);
    
#ifdef __cplusplus
}
#endif

#endif
