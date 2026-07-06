/*
 * commands.h
 *
 *  Created on: 28-Oct-2024
 *      Author: maheswaran_gurusamy
 */

#ifndef DEBUGCLI_COMMANDS_H_
#define DEBUGCLI_COMMANDS_H_

#include <stdbool.h>
#include <stdint.h>

#include "cliDebug.h"

/*******************************************************************
 *                      MACRO DEFINITION
 *******************************************************************/
#ifndef MAX_MAIN_CMD_LENGTH
#define MAX_MAIN_CMD_LENGTH  25
#endif

#ifndef MAX_NO_ARGV
#define MAX_NO_ARGV  4
#endif

#ifndef MAX_ARGV_CNT
#define MAX_ARGV_CNT  10
#endif

/*******************************************************************
 *                      TYPE DEFINITION
 *******************************************************************/
typedef void ( *CliCMDHandlingfn )( void );

typedef enum
{
    CMD_HELP,
    CMD_INFO,
    MAX_NO_CMD
} eCmdList_t;

typedef enum
{
    PROGRAMMER,
    TESTER,
    CUSTOMER,
    ENDUSER
} eAuthenticationLevel_t;

typedef struct cmdSet
{
    eCmdList_t              cmdID;
    char                    commandStr[ MAX_CMD_LENGTH ];
    CliCMDHandlingfn        cLiCmdHandler;
    eAuthenticationLevel_t  cmdPermissionLevel;
} CommandSet_t;

/*******************************************************************
 *                      FUNCTION PROTOTYPES
 *******************************************************************/
void executeCLiCommands( char *cmd );

#endif /* DEBUGCLI_COMMANDS_H_ */
