/*
 * commands.c
 *
 *  Created on: 28-Oct-2024
 *      Author: maheswaran_gurusamy
 */

#include <string.h>

#include "macro.h"
#include "commands.h"

/*******************************************************************
 *                      COMMAND HANDLER PROTOTYPES
 *******************************************************************/
void printHelp( void );
void printInfo( void );

/*******************************************************************
 *                      GLOBAL / MODULE VARIABLES
 *******************************************************************/
uint8_t argvCnt = ZERO;
char *argvPtr[ MAX_ARGV_CNT ] = { NULL };

const CommandSet_t cmdAvailble[ ] =
{
    { CMD_HELP, "help", printHelp, PROGRAMMER },
    { CMD_INFO, "info", printInfo, PROGRAMMER },
};

/*********************************************************************************
 *Name :- executeCLiCommands
 *Para1:- N/A
 *Return:-N/A
 *Details:- Execute the received command from CLi
 ******************************************************************************/
void executeCLiCommands( char *cmd )
{
    char *mainCMD = strtok( cmd, " " );
    uint8_t i = 0;

    argvCnt = ZERO;

    argvPtr[ argvCnt ] = strtok( NULL, " " );
    while ( argvPtr[ argvCnt ] != NULL )
    {
        argvPtr[ ++argvCnt ] = strtok( NULL, " " );
    }

    for ( i = 0; i < MAX_NO_CMD; i++ )
    {
        if ( false == mystrcmp( mainCMD, ( char* ) cmdAvailble[ i ].commandStr ) )
        {
            cmdAvailble[ i ].cLiCmdHandler();
            break;
        }
    }

    memset( argvPtr, 0, ( sizeof( char* ) * MAX_ARGV_CNT ) );
}

/*********************************************************************************
 *Name :- printHelp
 *Para1:- N/A
 *Return:-N/A
 *Details:- Need to consider future use for formated print
 ******************************************************************************/
void printHelp( void )
{
    debugText( "\n\rHELP command was received !!! \n\r" );
    debugText( "\n\rhelp\t---> Display this help text." );
    debugText( "\n\rinfo\t---> Display system Information." );
}

/*********************************************************************************
 *Name :- printInfo
 *Para1:- N/A
 *Return:-N/A
 *Details:- Just print the welcome note.
 ******************************************************************************/
void printInfo( void )
{
    cliPrintLabel();
}
