/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1993-1996  Alberto Pasquale                 */
/*                                                                           */
/*                   A L L   R I G H T S   R E S E R V E D                   */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/* This source code is NOT in the public domain and it CANNOT be used or     */
/* distributed without written permission from the author.                   */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/* How to contact the author:  Alberto Pasquale of 2:332/504@fidonet         */
/*                             alberto.pasquale@mo.nettuno.it                */
/*                             Viale Verdi 106                               */
/*                             41100 Modena                                  */
/*                             Italy                                         */
/*                                                                           */
/*****************************************************************************/

// ApCom.Hpp Ver. 00        // 32 bits only

#ifndef _APCOM_HPP_
#define _APCOM_HPP_

// The COM class is for handling a communication device.
// The interface is pretty general and capable of virtualizing
// any communication device (RS-232 ASYNC with modem, RS-232
// ASYNC with NULL modem, ISDN CAPI, etc.).

// Devices currently supported:
// -> RS-232 ASYNC with MODEM



           // Errors returned by COM::Error, bit mapped

#define NO_ERROR        0x00000000
#define BUFFER_OVERRUN  0x00000001
#define DEVICE_OVERRUN  0x00000002
#define PARITY_ERROR    0x00000004
#define FRAMING_ERROR   0x00000008

       // Errors returned by other functions (negative integers)
       // Positive integers are reserved for status information


#define CALL_REFUSED        2   // Incoming Ring not answered (can't reject)
#define RING_RCVD           1   // Incoming Ring during operation
#define OK                  0
#define TIMEOUT             -1
#define OPEN_ERR            -2
#define CLOSE_ERR           -3
#define MODE_ERR            -4
#define WRITE_ERR           -5
#define READ_ERR            -6
#define INIT_ERR            -7
#define DCD_ON_ERR          -8
#define DIAL_ERR            -9
#define INQUIRE_ERR         -10
#define ANSWER_ERR          -11
#define REJECT_ERR          -12
#define BUSY_ERR            -13
#define DEVCFG_ERR          -14


                //  For SetMode
                //  case insensitive, separated by commas;
                //  fields not to be changed may be empty.

// for RS-232 modem:

// first field:
//      DTE speed, decimal
//
//      Example: 115200
//
//      Default: current
//
// second field:
//      3 characters (parity, data bits, stop bits)
//      data bits: 5, 6, 7, 8
//      parity: N (None), O (Odd), E (Even), M (Mark), S (Space)
//      stop bits: 1, 1.5, 2
//
//      Example: 8N1 (8 data bits, No parity, 1 stop bit)
//
//      Default: 8N1


                // For DevCfg
                // case insensitive
                // <keyword> <string>


// Busy Reject|Ignore   // how to handle the calls when busy

// for RS-232 modem:

// Any keyword may be used multiple times.

// <string> is <match> [<format>]

// - <match> must match the first words of the modem response.
// - If <match> consists of multiple words, it
//   must be enclosed in quotes.
// - A match is valid if the character after <match> in the
//   modem response is NOT a letter a..z
//
// - <format> is a string of tokens separated by space.
// - Each token asks to perform one or more actions.
// - If an action cannot be done, it is skipped.
// - A pointer to the currently analyzed character in the modem
//   response is kept; it is initialized to the first character
//   of <format> at the beginning.
// - A normal token asks to find the sequence (not necessarily
//   a whole word) and move the pointer after it.
// - The following special sequences are defined (case sensitive);
//
//   %C get the CID
//   %L get the LID (required Local Identifier)
//   %P get the Protocol required for Connect
//   %R get the Connect Receive bps rate (numeric);
//   %T get the Connect Transmit bps rate (numeric);

// If either %R or %T are not defined, they are considered equal.

// These sequences start a conversion that ends at the natural end
// of the field (space in case of strings) or where specified by
// an attached string of the sequences described below.
// If you need spaces in the specification of end-conversion,
// then you must enclose the whole string in quotes.

// The following sequences allow to move the pointer and can
// be linked together (except fo %t that must be followed by
// space).
//
//   %b go to the beginning of line (<match>)
//   %e go to the end of line (terminating null)
//   %< Start going backward
//   %> Return moving forward
//   %:<x> where <x> is a hex digit: move <x> chars,
//     if <x> exceeds availability, as %b or %e
//   %t<s> string of optional terminating chars (<token>%t<s>)
//     Space is always considered included in <s>.
//     %t is used to obtain a whole-word match: the relevant
//     char is NOT taken as part of the specified token;
//     the pointer is advanced _after_ it.
//   %s move to first space character
//   %w move to first non-space character
//   %n wait and load next line of modem response
//   %% the % char

// <keyword> can be one of:

// Ok
// Connect
// Fax
// CallRejected
// Ring
// NoCarrier
// Error
// NoDialTone
// Busy
// NoAnswer
// Ringing
// Voice
// Info



// Example for Courier:

// Ok               OK
// Connect          CONNECT %R
// Fax              +FCO
// Ring             RING %L %n %n %n Nmbr = %C
// NoCarrier        "NO CARRIER"
// NoCarrier        +FHS
// Error            ERROR
// NoDialTone       "NO DIAL TONE"
// Busy             BUSY
// NoAnswer         "NO ANSWER"
// Ringing          RINGING
// Voice            VOICE
// Info             +FDM

// Example for CFOS:

// Ok               OK
// Connect          CONNECT %R ID= %C
// Ring             RING %C%t/ %L%t/ %P
// NoCarrier        "NO CARRIER"
// Error            ERROR
// NoDialTone       "NO DIALTONE"
// Busy             BUSY
// NoAnswer         "NO ANSWER"
// Ringing          RINGING




                        // for WaitResult

enum ComResult_t {
    Ok,             // Command accepted
    Connect,        // Connected
    Fax,            // Incoming call is for fax
    CallRejected,   // Outbound Call rejected
    Ring,           // Ring
    NoCarrier,      // Call interrupted, can't connect
    Error,          // Command or Device Error
    NoDialTone,     // Can't dial
    Busy,           // Destination busy
    NoAnswer,       // Destination does not answer
    Ringing,        // Destination received the call
    Voice,          // A voice answered
    Info,           // Informative only
    Unknown,        // Unknown device answer
    Garbage,        // Garbage device answer
    TimeOut         // Device timed out
};


typedef void (*INQF) (pcsz text);   // There is NO line termination in text.


class COM {
  private:

    HFILE   Comh;
    BYTE    ungetb;
    BOOL    ungetOk;
    CHAR    result[100];
    APIRET  SetDCB (void);   /* Set DCB Parameters */

  public:

    int     Open (pcsz dev,           // Device to be opened
                  pcsz mode = NULL);  // Device parameters, comma separated
            // returns: OK, OPEN_ERR, MODE_ERR

    int     Close ();   // device close
            // returns: OK, CLOSE_ERR

    int     Write (pcvoid buf,  // object to be written
                   uint len);   // size of object
            // returns: OK, WRITE_ERR

    int     Read (pvoid buf,            // buffer for incoming data
                  uint len,             // size of buffer
                  ulong timeout = 0);   // Max wait in sec/100
            // returns: bytes read, READ_ERR, TIMEOUT

    int     Puts (pcsz String);     // writes zero terminated string
            // returns: OK, WRITE_ERR

    int     Putb (byte c);          // writes byte c
            // returns: OK, WRITE_ERR

    int     Getb (byte *c,              // address to store byte
                  ulong timeout = 0);   // max wait in sec/100
            // returns: OK, READ_ERR, TIMEOUT

    void    UnGetb (byte b);    // rewind 1 byte

    int     Waitb (ulong timeout,  // max wait in sec/100
                   ...);      // list of bytes (int type) to wait for,
                              // terminated by -1.
            // returns: byte arrived, TIMEOUT

    void    FlushIn ();     // Flush Input Buffers

    void    FlushOut ();    // Flush Output Buffers

    uint    Error ();
            // Return and clears Com Device errors

    ulong   GetBps ();
            // returns Max Com speed (DTE for RS232)

    int     SetMode (pcsz mode);    // mode comma separated fields
            // returns n. of invalid field, 0 on success

    int     Init (pcsz InitString = NULL); // Init string for com dev
            // returns OK, INIT_ERR, DCD_ON_ERR

    int     Dial (pcsz Dest,            // Destination number
                  pcsz DialCmd = NULL); // Type of Dial required
            // returns: OK, DIAL_ERR, RING_RCVD

    int     Inquire (INQF inqf,         // INQF pointer for text answer.
                     pcsz InqCmd = NULL);  // Command for Inquiry
            // returns OK, INQUIRE_ERR, RING_RCVD

    ComResult_t Answer (pcsz AnsCmd = NULL);   // Answer cmd for com dev

    int     Reject (pcsz RejCmd = NULL);       // Reject cmd
            // returns OK, CALL_REFUSED, REJECT_ERR

    ComResult_t WaitResult (ulong timeout); // Max wait in s/100

    int     HangUp ();  // Release Call
            // returns: OK, DCD_ON_ERR

    int     Busy ();    // The Application is Busy
                        // Init to return active
            // returns: OK, BUSY_ERR

    pcsz    Handle ();
            // returns string representing handle of COM dev

    bool    Carrier ();
            // true when the comm link is active

    int     DevCfg (pcsz line);  // line of cfg for special
            // returns: OK, DEVCFG_ERR  // configuration of COM device;
                                        // NOT CR/LF terminated.

           // The Connect results returned by the following
           // functions are zeroed after Init,

    ulong   Bps ();         // Rx/Tx speed; Rx only if TxBps != 0
    ulong   TxBps ();       // If != 0, this is tx speed.
    bool    BidiExc ();     // ping/pong protocol

                            // The following functions return
                            // Empty string if no information available.

    pcsz    ResStr ();      // Last Result from Com Device
    pcsz    CID ();         // Caller IDentification
    pcsz    LID ();         // Local ID required (Distinctive Ring, EAZ)
    pcsz    Prtcl ();       // Protocol Required/Used
};


#endif
