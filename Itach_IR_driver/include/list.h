
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



/* ����������ͷ�����½ڵ�(�ڵ㲻��������)
		head   new(�ڵ�)	  2		
*/
/* Add new element at the head of the list.  */
static inline void
list_add (list_t *newp, list_t *head)
{
  head->next->prev = newp;		// 2����һ����new
  newp->next = head->next;		// new����һ����2
  newp->prev = head;			// new����һ����head
  head->next = newp;			// head����һ����new
}

/* �����Ԫ�ص�����β��
	head 	1	2	3 new
*/
/* Add new element at the tail of the list.  */
static inline void
list_add_tail (list_t *newp, list_t *head)
{
  head->prev->next = newp;	// 3����һ����new
  newp->next = head;		// new����һ��ʱhead
  newp->prev = head->prev;	// new����һ��ʱ3
  head->prev = newp;		// ͷ����һ��new
}

/* ɾ��ָ��Ԫ��:
	(1)   1 elem 2
	(2) ��������ֻ������ͷʱ, ����ͷû��ûɾ��(��Ϊ�ڵ㲻��������)
		head	// �Լ�����һ�����Լ�
				// �Լ�����һ��Ҳ���Լ�
*/
/* Remove element from list.  */
static inline void
list_del (list_t *elem)
{
  elem->next->prev = elem->prev;	// 2����һ����1
  elem->prev->next = elem->next;	// 1����һ��ʱ2
}

/* ƴ����������:
	1.�������������ֻ��һ������ͷ�Ͳ����κδ���(��Ϊ����ͷ����������)
	2.	(head 	 2) 	+	(add  4  5)	
		==> (head 	(5 	 6)   2   3)		// ��������ͷadd
*/
/* Join two lists.  */
static inline void
list_splice (list_t *add, list_t *head)
{
  /* Do nothing if the list which gets added is empty.  */
  if (add != add->next)
    {
      add->next->prev = head;		// 5����һ����head
      add->prev->next = head->next; // 6����һ����2
      head->next->prev = add->prev; // 2����һ����6
      head->next = add->next;		// head����һ����5
    }
}

// ���ݽڵ�ptr�ĵ�ַ,�õ���type�ṹ����׵�ַ, member��type�ṹ���еĽڵ������
/* Get typed element from list at a given position.  */
#define list_entry(ptr, type, member) \
  ((type *) ((char *) (ptr) - (unsigned long) (&((type *) 0)->member)))

#define container_of(ptr, type, member) \
	list_entry(ptr, type, member)
	
	
// �ӵ�һ���ڵ�(head����һ�ʼ, ������head�����е����нڵ�)
/* Iterate forward over the elements of the list.  */
#define list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

// �ӵ����һ���ڵ�(head����һ�ʼ, ��ǰ����head�����е����нڵ�)
/* Iterate forward over the elements of the list.  */
#define list_for_each_prev(pos, head) \
  for (pos = (head)->prev; pos != (head); pos = pos->prev)

// 1.�ӵ����һ���ڵ�(head����һ�ʼ, ��ǰ����head�����е����нڵ�)
// 2.��������������Ҫɾ���ڵ�ʱ, �ô˺���
// 3.���ʹ��list_for_each_prev, ��ɾ���ڵ��pos = pos->prev�����
/* Iterate backwards over the elements list.  The list elements can be
   removed from the list while doing this.  */
#define list_for_each_prev_safe(pos, p, head) \
  for (pos = (head)->prev, p = pos->prev; \
       pos != (head); \
       pos = p, p = pos->prev)

#endif	/* list.h */
