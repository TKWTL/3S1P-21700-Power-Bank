#include "fifo.h"

/*******************************8位长度FIFO系列********************************/
//返回FIFO内元素个数
uint8_t GetFIFOLength_Byte(struct FIFO_byte_t *p){
    return (p->head > p->tail)? p->head- p->tail- 1 : p->length- p->tail+ p->head;
}
//向FIFO内填充一个数据，位置为head
void FIFO_In_Byte(struct FIFO_byte_t *p, uint8_t data){
    if(GetFIFOLength_Byte(p) == p->length) return;
    p->content[p->head] = data;
    if(p->head == p->length) p->head = 0;
    else p->head++;
}
//从FIFO中取出一个数据，位置为tail+1
uint8_t FIFO_Out_Byte(struct FIFO_byte_t *p){
    if(GetFIFOLength_Byte(p) == 0) return 0;
    if(p->tail == p->length) p->tail = 0;
    else p->tail++;
    return p->content[p->tail];
}
//从FIFO头部向后读取指定偏移量的数据
uint8_t FIFO_QueryfromHead_Byte(struct FIFO_byte_t *p, uint8_t offset){
    if(offset+ 1 > GetFIFOLength_Byte(p)) return 0;
    if(p->head < offset+ 1) return p->content[p->length- offset+ p->head];
    else return p->content[p->head- 1- offset];
}
//从FIFO尾部向前读取指定偏移量的数据
uint8_t FIFO_QueryfromTail_Byte(struct FIFO_byte_t *p, uint8_t offset){
    if(offset > GetFIFOLength_Byte(p)) return 0;
    if(offset+ 1 > p->length- p->tail) return p->content[p->tail+ offset- p->length];
    else return p->content[p->tail+ 1+ offset];
}
//从FIFO头部向后读取指定偏移量的数据，不检查长度
uint8_t FIFO_QueryfromHead_Byte_NoCheck(struct FIFO_byte_t *p, uint8_t offset){
    if(p->head < offset+ 1) return p->content[p->length- offset+ p->head];
    else return p->content[p->head- 1- offset];
}
//从FIFO尾部向前读取指定偏移量的数据，不检查长度
uint8_t FIFO_QueryfromTail_Byte_NoCheck(struct FIFO_byte_t *p, uint8_t offset){
    if(offset+ 1 > p->length- p->tail) return p->content[p->tail+ offset- p->length];
    else return p->content[p->tail+ 1+ offset];
}
//从前向后删除FIFO内容，不取出数据，通过向后移动头指针实现
void FIFO_DeletefromHead_Byte(struct FIFO_byte_t *p, uint8_t num){
    if(num > GetFIFOLength_Byte(p)) return;
    if(p->head < num) p->head = p->length- num+ 1+ p->head;
    else p->head -= num;
}
//从后向前删除FIFO内容，不取出数据，通过向前移动尾指针实现
void FIFO_DeletefromTail_Byte(struct FIFO_byte_t *p, uint8_t num){
    if(num > GetFIFOLength_Byte(p)) return;
    if(p->tail > p->length- num) p->tail = p->tail+ num- p->length- 1;
    else p->tail += num;
}

/*******************************16位长度FIFO系列********************************/
//返回FIFO内元素个数
uint16_t GetFIFOLength_Word(struct FIFO_word_t *p){
    return (p->head > p->tail)? p->head- p->tail- 1 : p->length- p->tail+ p->head;
}
//向FIFO内填充一个数据，位置为head，并将指针移动到下一个位置
void FIFO_In_Word(struct FIFO_word_t *p, uint8_t data){
    if(GetFIFOLength_Word(p) == p->length) return;
    p->content[p->head] = data;
    if(p->head == p->length) p->head = 0;
    else p->head++;
}
//从FIFO中取出一个数据，位置为tail+1
uint8_t FIFO_Out_Word(struct FIFO_word_t *p){
    if(GetFIFOLength_Word(p) == 0) return 0;
    if(p->tail == p->length) p->tail = 0;
    else p->tail++;
    return p->content[p->tail];
}
//从FIFO头部向后读取指定偏移量的数据
uint8_t FIFO_QueryfromHead_Word(struct FIFO_word_t *p, uint16_t offset){
    if(offset+ 1 > GetFIFOLength_Word(p)) return 0;
    if(p->head < offset+ 1) return p->content[p->length- offset+ p->head];
    else return p->content[p->head- 1- offset];
}
//从FIFO尾部向前读取指定偏移量的数据
uint8_t FIFO_QueryfromTail_Word(struct FIFO_word_t *p, uint16_t offset){
    if(offset > GetFIFOLength_Word(p)) return 0;
    if(offset+ 1 > p->length- p->tail) return p->content[p->tail+ offset- p->length];
    else return p->content[p->tail+ 1+ offset];
}
//从FIFO头部向后读取指定偏移量的数据，不检查长度
uint8_t FIFO_QueryfromHead_Word_NoCheck(struct FIFO_word_t *p, uint16_t offset){
    if(p->head < offset+ 1) return p->content[p->length- offset+ p->head];
    else return p->content[p->head- 1- offset];
}
//从FIFO尾部向前读取指定偏移量的数据，不检查长度
uint8_t FIFO_QueryfromTail_Word_NoCheck(struct FIFO_word_t *p, uint16_t offset){
    if(offset+ 1 > p->length- p->tail) return p->content[p->tail+ offset- p->length];
    else return p->content[p->tail+ 1+ offset];
}
//从前向后删除FIFO内容，不取出数据，通过向后移动头指针实现
void FIFO_DeletefromHead_Word(struct FIFO_word_t *p, uint16_t num){
    if(num > GetFIFOLength_Word(p)) return;
    if(p->head < num) p->head = p->length- num+ 1+ p->head;
    else p->head -= num;
}
//从后向前删除FIFO内容，不取出数据，通过向前移动尾指针实现
void FIFO_DeletefromTail_Word(struct FIFO_word_t *p, uint16_t num){
    if(num > GetFIFOLength_Word(p)) return;
    if(p->tail > p->length- num) p->tail = p->tail+ num- p->length- 1;
    else p->tail += num;
}
