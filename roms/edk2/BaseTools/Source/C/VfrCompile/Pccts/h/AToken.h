/* ANTLRToken.h
 *
 * SOFTWARE RIGHTS
 *
 * We reserve no LEGAL rights to the Purdue Compiler Construction Tool
 * Set (PCCTS) -- PCCTS is in the public domain.  An individual or
 * company may do whatever they wish with source code distributed with
 * PCCTS or the code generated by PCCTS, including the incorporation of
 * PCCTS, or its output, into commerical software.
 *
 * We encourage users to develop software with PCCTS.  However, we do ask
 * that credit is given to us for developing PCCTS.  By "credit",
 * we mean that if you incorporate our source code into one of your
 * programs (commercial product, research project, or otherwise) that you
 * acknowledge this fact somewhere in the documentation, research report,
 * etc...  If you like PCCTS and have developed a nice tool with the
 * output, please mention that you developed it using PCCTS.  In
 * addition, we ask that this header remain intact in our source code.
 * As long as these guidelines are kept, we expect to continue enhancing
 * this system and expect to make other tools available as they are
 * completed.
 *
 * ANTLR 1.33
 * Terence Parr
 * Parr Research Corporation
 * with Purdue University and AHPCRC, University of Minnesota
 * 1989-2000
 */

#ifndef ATOKEN_H_GATE
#define ATOKEN_H_GATE

#include "pcctscfg.h"

#include "pccts_string.h"
#include "pccts_stdio.h"
#include "pccts_stdlib.h"
#include "pccts_stdarg.h" // MR23

PCCTS_NAMESPACE_STD

// MR9      RJV (JVincent@novell.com) Not needed for variable length strings

//// MR9 #ifndef ANTLRCommonTokenTEXTSIZE
//// MR9 #define ANTLRCommonTokenTEXTSIZE        	100
//// MR9 #endif


/* must define what a char looks like; can make this a class too */
typedef char ANTLRChar;

/* D E F I N E  S M A R T  P O I N T E R S */

//#include ATOKPTR_H   not tested yet, leave out
class ANTLRAbstractToken;
typedef ANTLRAbstractToken *_ANTLRTokenPtr;

class ANTLRAbstractToken {
public:
    virtual ~ANTLRAbstractToken() {;}
    virtual ANTLRTokenType getType() const = 0;
    virtual void setType(ANTLRTokenType t) = 0;
    virtual int getLine() const = 0;
    virtual void setLine(int line) = 0;
    virtual ANTLRChar *getText() const = 0;
    virtual void setText(const ANTLRChar *) = 0;

    /* This function will disappear when I can use templates */
	virtual ANTLRAbstractToken *makeToken(ANTLRTokenType tt,
										  ANTLRChar *text,
										  int line) = 0;

	/* define to satisfy ANTLRTokenBuffer's need to determine whether or
	   not a token object can be destroyed.  If nref()==0, no one has
	   a reference, and the object may be destroyed.  This function defaults
	   to 1, hence, if you use deleteTokens() message with a token object
	   not derived from ANTLRCommonRefCountToken, the parser will compile
	   but will not delete objects after they leave the token buffer.
    */

	virtual unsigned nref() const { return 1; }     // MR11
	virtual void ref() {;}
	virtual void deref() {;}

	virtual void panic(const char *msg)             // MR20 const
		{
			/* MR23 */ printMessage(stderr, "ANTLRAbstractToken panic: %s\n", msg);
			exit(PCCTS_EXIT_FAILURE);
		}

	virtual int printMessage(FILE* pFile, const char* pFormat, ...) // MR23
		{
			va_list marker;
			va_start( marker, pFormat );
  			int iRet = vfprintf(pFile, pFormat, marker);
			va_end( marker );
			return iRet;
		}
};

/* This class should be subclassed.  It cannot store token type or text */

class ANTLRRefCountToken : public ANTLRAbstractToken {
public:
#ifdef DBG_REFCOUNTTOKEN
	static int ctor;
	static int dtor;
#endif
protected:
    unsigned refcnt_;
#ifdef DBG_REFCOUNTTOKEN
	char object[200];
#endif

public:

	// MR23 - No matter what you do, you're hammered.
	//        Don't give names to formals something breaks.
	//		  Give names to formals and don't use them it breaks.

#ifndef DBG_REFCOUNTTOKEN
	ANTLRRefCountToken(ANTLRTokenType /* t MR23 */, const ANTLRChar * /* s MR23 */)
#else
	ANTLRRefCountToken(ANTLRTokenType t, const ANTLRChar * s)
#endif

#ifndef DBG_REFCOUNTTOKEN
		{
			refcnt_ = 0;
		}
#else
	{
		ctor++;
		refcnt_ = 0;
		if ( t==1 ) sprintf(object,"tok_EOF");
		else sprintf(object,"tok_%s",s);
		/* MR23 */ printMessage(stderr, "ctor %s #%d\n",object,ctor);
	}
#endif
	ANTLRRefCountToken()
#ifndef DBG_REFCOUNTTOKEN
		{ refcnt_ = 0; }
#else
		{
			ctor++;
			refcnt_ = 0;
			sprintf(object,"tok_blank");
			/* MR23 */ printMessage(stderr, "ctor %s #%d\n",object,ctor);
		}
	virtual ~ANTLRRefCountToken()
		{
			dtor++;
			if ( dtor>ctor ) /* MR23 */ printMessage(stderr, "WARNING: dtor>ctor\n");
			/* MR23 */ printMessage(stderr, "dtor %s #%d\n", object, dtor);
			object[0]='\0';
		}
#endif

	// reference counting stuff needed by ANTLRTokenPtr.
	// User should not access these; for C++ language reasons, we had
	// to make these public.  Yuck.

	void ref()		      { refcnt_++; }
	void deref()	      { refcnt_--; }
	unsigned nref()	const { return refcnt_; }   // MR11

	virtual ANTLRAbstractToken *makeToken(ANTLRTokenType /*tt MR23*/,
										  ANTLRChar * /*txt MR23*/,
										  int /*line MR23*/)
	{
		panic("call to ANTLRRefCountToken::makeToken()\n");
		return NULL;
	}
};

class ANTLRCommonNoRefCountToken : public ANTLRAbstractToken {
protected:
	ANTLRTokenType _type;
	int _line;
	ANTLRChar *_text;               // MR9 RJV

public:
	ANTLRCommonNoRefCountToken(ANTLRTokenType t, const ANTLRChar *s)
	{ setType(t); _line = 0; _text = NULL; setText(s); }
	ANTLRCommonNoRefCountToken()
	{ setType((ANTLRTokenType)0); _line = 0; _text = NULL; setText(""); }

	~ANTLRCommonNoRefCountToken() { if (_text) delete [] _text; }  // MR9 RJV: Added Destructor to remove string

	ANTLRTokenType getType() const 	{ return _type; }
	void setType(ANTLRTokenType t)	{ _type = t; }
	virtual int getLine() const		{ return _line; }
	void setLine(int line)	    	{ _line = line; }
	ANTLRChar *getText() const   	{ return _text; }
    int getLength() const           { return strlen(getText()); }       // MR11

// MR9 RJV: Added code for variable length strings to setText()

	void setText(const ANTLRChar *s)
	{	if (s != _text) {
          if (_text) delete [] _text;
          if (s != NULL) {
         	_text = new ANTLRChar[strlen(s)+1];
            if (_text == NULL) panic("ANTLRCommonNoRefCountToken::setText new failed");
            strcpy(_text,s);
    	  } else {
            _text = new ANTLRChar[1];
            if (_text == NULL) panic("ANTLRCommonNoRefCountToken::setText new failed");
            strcpy(_text,"");
          };
        };
	}

	virtual ANTLRAbstractToken *makeToken(ANTLRTokenType tt,
										  ANTLRChar *txt,
										  int line)
		{
			ANTLRAbstractToken *t = new ANTLRCommonNoRefCountToken;
			t->setType(tt); t->setText(txt); t->setLine(line);
			return t;
		}

// MR9 THM Copy constructor required when heap allocated string is used with copy semantics

   ANTLRCommonNoRefCountToken (const ANTLRCommonNoRefCountToken& from) :
         ANTLRAbstractToken(from) {
 	 setType(from._type);
	 setLine(from._line);
     _text=NULL;
     setText(from._text);
  };

// MR9 THM operator =() required when heap allocated string is used with copy semantics

   virtual ANTLRCommonNoRefCountToken& operator =(const ANTLRCommonNoRefCountToken& rhs) {

//////  MR15 WatCom can't hack use of operator =()
//////  Use this:  *( (ANTRLAbstractToken *) this)=rhs;

     *( (ANTLRAbstractToken *) this ) = rhs;

  	 setType(rhs._type);
 	 setLine(rhs._line);
     setText(rhs._text);
     return *this;
   };
};

class ANTLRCommonToken : public ANTLRRefCountToken {
protected:
	ANTLRTokenType       _type;
	int                  _line;
	ANTLRChar           *_text;               // MR9 RJV:Added

public:
	ANTLRCommonToken(ANTLRTokenType t, const ANTLRChar *s) : ANTLRRefCountToken(t,s)
		{ setType(t); _line = 0; _text = NULL; setText(s); }                    // MR9
	ANTLRCommonToken()
		{ setType((ANTLRTokenType)0); _line = 0; _text = NULL; setText(""); }   // MR9

	virtual ~ANTLRCommonToken() { if (_text) delete [] _text; } // MR9 RJV: Added Destructor to remove string

	ANTLRTokenType getType() const 	{ return _type; }
	void setType(ANTLRTokenType t)	{ _type = t; }
	virtual int getLine() const		{ return _line; }
	void setLine(int line)	    	{ _line = line; }
	ANTLRChar *getText() const		{ return _text; }
    int getLength() const           { return strlen(getText()); }       // MR11

// MR9 RJV: Added code for variable length strings to setText()

	void setText(const ANTLRChar *s)
	{	if (s != _text) {
          if (_text) delete [] _text;
          if (s != NULL) {
         	_text = new ANTLRChar[strlen(s)+1];
            if (_text == NULL) panic("ANTLRCommonToken::setText new failed");
            strcpy(_text,s);
    	  } else {
            _text = new ANTLRChar[1];
            if (_text == NULL) panic("ANTLRCommonToken::setText new failed");
            strcpy(_text,"");
          };
        };
	}

	virtual ANTLRAbstractToken *makeToken(ANTLRTokenType tt,
										  ANTLRChar *txt,
										  int line)
	{
		ANTLRAbstractToken *t = new ANTLRCommonToken(tt,txt);
		t->setLine(line);
		return t;
	}

// MR9 THM Copy constructor required when heap allocated string is used with copy semantics

   ANTLRCommonToken (const ANTLRCommonToken& from) :
         ANTLRRefCountToken(from) {
 	 setType(from._type);
	 setLine(from._line);
     _text=NULL;
     setText(from._text);
  };

// MR9 THM operator =() required when heap allocated string is used with copy semantics

   virtual ANTLRCommonToken& operator =(const ANTLRCommonToken& rhs) {

//////  MR15 WatCom can't hack use of operator =()
//////  Use this instead:   *( (ANTRLRRefCountToken *) this)=rhs;

     *( (ANTLRRefCountToken *) this) = rhs;

  	 setType(rhs._type);
 	 setLine(rhs._line);
     setText(rhs._text);
     return *this;
   };
};

// used for backward compatibility
typedef ANTLRCommonToken ANTLRCommonBacktrackingToken;

#endif
