
#ifndef _LIST_H
#define _LIST_H	

#include <stdbool.h>

typedef struct list_head
{
  struct list_head *next;
  struct list_head *prev;
} list_t;




#define LIST_HEAD(name) \
  list_t name = { &(name), &(name) }


/* Initialize a new list head.  */
#define INIT_LIST_HEAD(ptr) \
  (ptr)->next = (ptr)->prev = (ptr)



/* 紧挨着链表头增加新节点(节点不包含数据)
		head   new(节点)	  2		
*/
/* Add new element at the head of the list.  */
static inline void
list_add (list_t *newp, list_t *head)
{
  head->next->prev = newp;		// 2的上一项是new
  newp->next = head->next;		// new的下一项是2
  newp->prev = head;			// new的上一项是head
  head->next = newp;			// head的下一项是new
}

/* 添加新元素到链表尾部
	head 	1	2	3 new
*/
/* Add new element at the tail of the list.  */
static inline void
list_add_tail (list_t *newp, list_t *head)
{
  head->prev->next = newp;	// 3的下一项是new
  newp->next = head;		// new的下一项时head
  newp->prev = head->prev;	// new的上一项时3
  head->prev = newp;		// 头的上一项new
}

/* 删除指定元素:
	(1)   1 elem 2
	(2) 当链表中只有链表头时, 链表头没有没删除(因为节点不包含数据)
		head	// 自己的上一项是自己
				// 自己的下一项也是自己
*/
/* Remove element from list.  */
static inline void
list_del (list_t *elem)
{
  elem->next->prev = elem->prev;	// 2的上一项是1
  elem->prev->next = elem->next;	// 1的下一项时2
}

/* 拼接两个链表:
	1.如果新增的链表只有一个链表头就不做任何处理(因为链表头不包含数据)
	2.	(head 	 2) 	+	(add  4  5)	
		==> (head 	(5 	 6)   2   3)		// 抛弃链表头add
*/
/* Join two lists.  */
static inline void
list_splice (list_t *add, list_t *head)
{
  /* Do nothing if the list which gets added is empty.  */
  if (add != add->next)
    {
      add->next->prev = head;		// 5的上一项是head
      add->prev->next = head->next; // 6的下一项是2
      head->next->prev = add->prev; // 2的上一项是6
      head->next = add->next;		// head的下一项是5
    }
}

// 根据节点ptr的地址,得到其type结构体的首地址, member是type结构体中的节点变量名
/* Get typed element from list at a given position.  */
#define list_entry(ptr, type, member) \
  ((type *) ((char *) (ptr) - (unsigned long) (&((type *) 0)->member)))

#define container_of(ptr, type, member) \
	list_entry(ptr, type, member)
	
	
// 从第一个节点(head的下一项开始, 向后遍历head链表中的所有节点)
/* Iterate forward over the elements of the list.  */
#define list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

// 从第最后一个节点(head的上一项开始, 向前遍历head链表中的所有节点)
/* Iterate forward over the elements of the list.  */
#define list_for_each_prev(pos, head) \
  for (pos = (head)->prev; pos != (head); pos = pos->prev)

// 1.从第最后一个节点(head的上一项开始, 向前遍历head链表中的所有节点)
// 2.当遍历过程中需要删除节点时, 用此函数
// 3.如果使用list_for_each_prev, 在删除节点后pos = pos->prev会出错
/* Iterate backwards over the elements list.  The list elements can be
   removed from the list while doing this.  */
#define list_for_each_prev_safe(pos, p, head) \
  for (pos = (head)->prev, p = pos->prev; \
       pos != (head); \
       pos = p, p = pos->prev)

#endif	/* list.h */
