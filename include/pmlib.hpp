/*****************************************************************************/
/*                                                                           */
/*                 (C) Copyright 1995-1997  Alberto Pasquale                 */
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
/*   How to contact the author:  Alberto Pasquale of 2:332/504@fidonet       */
/*                               Viale Verdi 106                             */
/*                               41100 Modena                                */
/*                               Italy                                       */
/*                                                                           */
/*****************************************************************************/

// PmLib.Hpp 02

// REQUIRES:
//
// #define INCL_PM
// #include <os2.h>
//


#include <typedefs.h>


                // implementation in Win.Cpp


void WinCenter (HWND ref, HWND hwnd);

// window hwnd is centered with reference to window ref


                // Implementation in TaskList.Cpp

class TaskList {
    private:
        HAB hab;        // Application's anchor block for PM
        byte *buf;
        SWBLOCK *swb;
        void Delete ();
    public:
        TaskList (HAB hab); // Anchor Block of the application's PM interface
        ulong ReadList ();  // reads switch list, returns # of entries
        pcsz GetName (int i);    // Switch-list title
        HWND GetHwnd (int i);    // Window handle
        ~TaskList ();
};



