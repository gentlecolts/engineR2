#ifndef VNODE_H_INCLUDED
#define VNODE_H_INCLUDED

#include <SDL/SDL.h>
#include "../cam/vec3d.h"

#define linkparent 1

struct nodelist;

struct vnode{
	static uint32_t defcol;

	#if linkparent
	nodelist* nodes;
	#else
	vnode* next;
	#endif
	uint32_t color;
	/**
	shape: 8 bits representing which children exist
	index: 3 bits representing the index of the current node (used for finding the parent pointer)
	TODO: find a use for the 21 unused bits
	*/
	uint8_t shape,index;
	uint16_t unused;

	vnode(uint8_t shp=0x00,uint32_t col=defcol);
	vnode(const vnode& node);
	virtual void operator =(const vnode& node);
	virtual ~vnode();
	virtual void die();

	#if linkparent
	virtual vnode* getParent();
	#endif

	virtual void initChildren(uint8_t node_shape=0xff);
	virtual void deleteChildren();
	virtual void deleteChild(int i);

	//TODO: combine these two functions
	virtual uint32_t calcColors();
	virtual void cleanTree();

	/*if security becomes a concern, then make next private/protected and use these functions
	the biggest concern being someone makes next point to an array which is not vnode[8]

	another solution would be to make next an array of 8 pointers:
	this would be good because unless i am wrong, this means its size cannot be changed
	though if the array can still be reallocated/resized then it removes some of the advantage of doing this,
	which would mean that i have to make next private and might mean that i would need to remove the ability to subclass this struct

	pros
	-would remove the need to check for next being null
	-removes some of the security problem
	-dont need to store shape variable
	-dont need to allocate all 8 children at a time

	cons
	-each node takes up more memory (instead of storing 1 pointer, they now store 8, which is an even bigger issue on 64bit machines)
	-need to check each child pointer for being null
	-removal/creation of nodes changes from fliping a bit to (de)allocation of an object (unless i still store shape)
	-not storing shape removes lazy deallocation
		(set the flag bit to 0, but dont actually remove its children until the clean step, which could either be automatic or a function of vobj)
		having it be a function call also allows you to hide a node and its children by setting its flag in shape to zero
	-the pointers in the array might still be abusable
	*/
	/*
	void createChildren();
	void deleteChild(int i);
	void deleteChildren();

	vnode* getChild(int i);
	//*/
};

struct nodelist{
	const vnode* selfpntr;
	vnode next[8];
	nodelist(vnode* node);
};

#endif // VNODE_H_INCLUDED
