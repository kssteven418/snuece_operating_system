#include <core/eos.h>
#include <core/eos_internal.h>

typedef struct _os_context {
	/* low address */
	int32u_t edi;
	int32u_t esi;
	int32u_t ebp;
	int32u_t esp;
	int32u_t ebx;
	int32u_t edx;
	int32u_t ecx;
	int32u_t eax;
	int32u_t eflags;
	int32u_t eip;
	/* high address */	
} _os_context_t;

static _os_context_t context;

void print_context(addr_t context) {
	if(context == NULL) return;
	_os_context_t *ctx = (_os_context_t *)context;
	PRINT("edi  =0x%x\n", ctx->edi);
	PRINT("esi  =0x%x\n", ctx->esi);
	PRINT("ebp  =0x%x\n", ctx->ebp);
	PRINT("esp  =0x%x\n", ctx->esp);
	PRINT("ebx  =0x%x\n", ctx->ebx);
	PRINT("edx  =0x%x\n", ctx->edx);
	PRINT("ecx  =0x%x\n", ctx->ecx);
	PRINT("eax  =0x%x\n", ctx->eax);
	PRINT("eflags  =0x%x\n", ctx->eflags);
	PRINT("eip  =0x%x\n", ctx->eip);
	
}

addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *), void *arg) {
	
	// push arg first
	stack_base -= sizeof(arg);
	*(int32u_t*)(stack_base) = (int32u_t)arg;

	//push return address : entry
	stack_base -= sizeof(void*);
	*(int32u_t*)(stack_base) = (int32u_t)entry;
	
	//push null ebp
	stack_base -= sizeof(void*);
	*(int32u_t*)(stack_base) = (int32u_t)0;
	
	//push context
	stack_base -= sizeof(context);
	
	return stack_base;
}

void _os_restore_context(addr_t sp) {
	
	__asm__ __volatile("movl %0, %%esp;\
						popa;\
						popf;\
						movl %%eax, %1;\
						pop %%eax;\
						movl %2, %%eax"
						:: "m"(sp), "m"(context.eax), "m"(context.eax));
}

addr_t _os_save_context() {
	
	//get ebp and eip
	__asm__ __volatile("movl %%eax, %0;\
						pop %%eax;\
						movl %%eax, %1;\
						pop %%eax;\
						movl %%eax, %2;\
						add $0x1c, %%eax;\
						push %%eax;\
						movl %3, %%eax;\
						push %%eax;\
						movl %4, %%eax"
						::"m"(context.eax), "m"(context.ebp), "m"(context.eip), 
						"m"(context.ebp), "m"(context.eax));	

	/* push context*/

	//push dummy eip with eax
	//push flag
	//push remaining registers
	//set stack pointer as a return value
	//push dummy space

	__asm__ __volatile("push %%eax;\
						pushf;\
						pusha;\
						movl %%esp, %0;\
						push %%eax;\
						push %%eax;\
						push %%eax;\
						push %%eax;"
						:: "m"(context.esp));
	
	//push eip and ebp
	__asm__ __volatile("movl %0, %%eax;\
						push %%eax;\
						movl %1, %%eax;\
						push %%eax;\
						movl %%esp, %%ebp"::
						"m"(context.eip), "m"(context.ebp));
	return (addr_t)context.esp;

}
