/*This code copyrighted (2013) for the Warzone 2100 Legacy Project under the GPLv2.

Warzone 2100 Legacy is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Warzone 2100 Legacy is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Warzone 2100 Legacy; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA*/
/*
 * Interp.h
 *
 * Script interpreter definitions
 */
#ifndef _interp_h
#define _interp_h

/* The type of function called by an OP_CALL */
typedef BOOL (*SCRIPT_FUNC)(void);

/* The type of function called to access an object or in-game variable */
typedef BOOL (*SCRIPT_VARFUNC)(uint32_t index);

/* The possible value types for scripts */
typedef enum _interp_type
{
	// Basic types
	VAL_BOOL,
	VAL_INT,
	VAL_FLOAT,
	VAL_STRING,

	// events and triggers
	VAL_TRIGGER, //!< trigger index
	VAL_EVENT, //!< event (or in-script function)

	/* Type used by the compiler for functions that do not return a value */
	VAL_VOID,

	VAL_OPCODE, //!< Script opcode
	VAL_PKOPCODE, //!< Packed script opcode

	VAL_OBJ_GETSET, //!< Pointer to the object .set or .get function
	VAL_FUNC_EXTERN, //!< External function pointer

	VAL_USERTYPESTART, //!< user defined types should start with this id
	VAL_REF = 0x00100000 //!< flag to specify a variable reference rather than simple value
} INTERP_TYPE;


/* A value consists of its type and value */
typedef struct _interp_val
{
	INTERP_TYPE		type;					//Value type for interpreter; opcode or value type for compiler
	union
	{
		char					*sval;			//String value - VAL_STRING
		SCRIPT_VARFUNC		pObjGetSet;		//Set/Get method for objects - VAL_OBJ_GETSET
		SCRIPT_FUNC			pFuncExtern;		//External (C) function - VAL_FUNC_EXTERN
		void					*oval;			//Object value - any in-game object
		float					fval;				//Float value - VAL_FLOAT
		int						ival;				// Integer value - VAL_INT
		BOOL					bval;			//Boolean value - VAL_BOOL
	} v;
} INTERP_VAL;


// maximum number of equivalent types for a type
#define INTERP_MAXEQUIV		10

// type equivalences
typedef struct _interp_typeequiv
{
	INTERP_TYPE		base;		// the type that the others are equivalent to
	unsigned int		numEquiv;	// number of equivalent types
	INTERP_TYPE		aEquivTypes[INTERP_MAXEQUIV]; // the equivalent types
} TYPE_EQUIV;

/* Opcodes for the script interpreter */
typedef enum _op_code
{
	OP_PUSH,		// Push value onto stack
	OP_PUSHREF,		// Push a pointer to a variable onto the stack
	OP_POP,			// Pop value from stack

	OP_PUSHGLOBAL,	// Push the value of a global variable onto the stack
	OP_POPGLOBAL,	// Pop a value from the stack into a global variable

	OP_PUSHARRAYGLOBAL,	// Push the value of a global array variable onto the stack
	OP_POPARRAYGLOBAL,	// Pop a value from the stack into a global array variable

	OP_CALL,		// Call the 'C' function pointed to by the next value
	OP_VARCALL,		// Call the variable access 'C' function pointed to by the next value

	OP_JUMP,		// Jump to a different location in the script
	OP_JUMPTRUE,	// Jump if the top stack value is true
	OP_JUMPFALSE,	// Jump if the top stack value is false

	OP_BINARYOP,	// Call a binary maths/boolean operator
	OP_UNARYOP,		// Call a unary maths/boolean operator

	OP_EXIT,			// End the program
	OP_PAUSE,			// temporarily pause the current event

	// The following operations are secondary data to OP_BINARYOP and OP_UNARYOP

	// Maths operators
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_NEG,
	OP_INC,
	OP_DEC,

	// Boolean operators
	OP_AND,
	OP_OR,
	OP_NOT,

	//String concatenation
	OP_CONC,

	// Comparison operators
	OP_EQUAL,
	OP_NOTEQUAL,
	OP_GREATEREQUAL,
	OP_LESSEQUAL,
	OP_GREATER,
	OP_LESS,

	OP_FUNC,		//custom (in-script) function call
	OP_POPLOCAL,	//local var
	OP_PUSHLOCAL,

	OP_PUSHLOCALREF,	//variable of object type (pointer)
	OP_TO_FLOAT,			//float cast
	OP_TO_INT,				//int cast
} OPCODE;

/* How far the opcode is shifted up a uint32_t to allow other data to be
 * stored in the same uint32_t
 */
#define OPCODE_SHIFT		24
#define OPCODE_DATAMASK		0x00ffffff

// maximum sizes for arrays
#define VAR_MAX_DIMENSIONS		4
#define VAR_MAX_ELEMENTS		uint8_t_MAX

/* The mask for the number of array elements stored in the data part of an opcode */
#define ARRAY_BASE_MASK			0x000fffff
#define ARRAY_DIMENSION_SHIFT	20
#define ARRAY_DIMENSION_MASK	0x00f00000

/* The possible storage types for a variable */
typedef enum _storage_type
{
	ST_PUBLIC,		// Public variable
	ST_PRIVATE,		// Private variable
	ST_OBJECT,		// A value stored in an objects data space.
	ST_EXTERN,		// An external value accessed by function call
	ST_LOCAL,		// A local variable
} enum_STORAGE_TYPE;

typedef uint8_t STORAGE_TYPE;

/* Variable debugging info for a script */
typedef struct _var_debug
{
	char			*pIdent;
	STORAGE_TYPE	storage;
} VAR_DEBUG;

/* Array info for a script */
typedef struct _array_data
{
	uint32_t			base;			// the base index of the array values
	INTERP_TYPE		type;			// the array data type
	uint8_t			dimensions;
	uint8_t			elements[VAR_MAX_DIMENSIONS];
} ARRAY_DATA;

/* Array debug info for a script */
typedef struct _array_debug
{
	char			*pIdent;
	uint8_t			storage;
} ARRAY_DEBUG;

/* Line debugging information for a script */
typedef struct _script_debug
{
	uint32_t	offset;		// Offset in the compiled script that corresponds to
	uint32_t	line;		// this line in the original script.
	char	*pLabel;	// the trigger/event that starts at this line
} SCRIPT_DEBUG;

/* Different types of triggers */
typedef enum _trigger_type
{
	TR_INIT,		// Trigger fires when the script is first run
	TR_CODE,		// Trigger uses script code
	TR_WAIT,		// Trigger after a time pause
	TR_EVERY,		// Trigger at repeated intervals
	TR_PAUSE,		// Event has paused for an interval and will restart in the middle of it's code

	TR_CALLBACKSTART,	// The user defined callback triggers should start with this id
} TRIGGER_TYPE;

/* Description of a trigger for the SCRIPT_CODE */
typedef struct _trigger_data
{
	TRIGGER_TYPE		type;		// Type of trigger
	uint16_t			code;		// BOOL - is there code with this trigger
	uint32_t			time;		// How often to check the trigger
} TRIGGER_DATA;

/* A compiled script and its associated data */
typedef struct _script_code
{
	uint32_t			size;			// The size (in bytes) of the compiled code
	INTERP_VAL		*pCode;			// Pointer to the compiled code

	uint16_t			numTriggers;	// The number of triggers
	uint16_t			numEvents;		// The number of events
	uint16_t			*pTriggerTab;	// The table of trigger offsets
	TRIGGER_DATA	*psTriggerData;	// The extra info for each trigger
	uint16_t			*pEventTab;		// The table of event offsets
	int16_t			*pEventLinks;	// The original trigger/event linkage
	// -1 for no link

	uint16_t			numGlobals;		// The number of global variables
	uint16_t			numArrays;		// the number of arrays in the program
	uint32_t			arraySize;		// the number of elements in all the defined arrays
	INTERP_TYPE		*pGlobals;		// Types of the global variables


	INTERP_TYPE		**ppsLocalVars;		//storage for local vars (type)
	uint32_t			*numLocalVars;		//number of local vars each event has
	INTERP_VAL		**ppsLocalVarVal;	//Values of the local vars used during interpreting process
	uint32_t			*numParams;			//number of arguments this event has


	VAR_DEBUG		*psVarDebug;	// The names and storage types of variables
	ARRAY_DATA		*psArrayInfo;	// The sizes of the program arrays
	ARRAY_DEBUG		*psArrayDebug;	// Debug info for the arrays

	uint16_t			debugEntries;	// Number of entries in psDebug
	SCRIPT_DEBUG	*psDebug;		// Debugging info for the script
} SCRIPT_CODE;


/* What type of code should be run by the interpreter */
typedef enum _interp_runtype
{
	IRT_TRIGGER,					// Run trigger code
	IRT_EVENT,						// Run event code
} INTERP_RUNTYPE;


/* The size of each opcode */
extern int32_t aOpSize[];

/* Check if two types are equivalent */
extern BOOL interpCheckEquiv(INTERP_TYPE to, INTERP_TYPE from) WZ_DECL_PURE;

// Initialise the interpreter
extern BOOL interpInitialise(void);

// true if the interpreter is currently running
extern BOOL interpProcessorActive(void);

/* Output script call stack trace */
extern void scrOutputCallTrace(code_part part);

#endif


