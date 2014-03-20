/***************************************************************************
* Program:
*    Linux Terminal Color Codes
* Author:
*    Bryson Gibbons
* Summary:
*    Senior Project
*    
***************************************************************************/

#ifndef _ERROR_COLORS_H
#define _ERROR_COLORS_H
char    RCol[]="\033[0m";     // Text Reset
char     Bla[]="\033[0;30m";  // Black  Text, Regular
char     Red[]="\033[0;31m";  // Red    Text, Regular
char     Gre[]="\033[0;32m";  // Green  Text, Regular
char     Yel[]="\033[0;33m";  // Yellow Text, Regular
char     Blu[]="\033[0;34m";  // Blue   Text, Regular
char     Pur[]="\033[0;35m";  // Purple Text, Regular
char     Cya[]="\033[0;36m";  // Cyan   Text, Regular
char     Whi[]="\033[0;37m";  // White  Text, Regular
char    BBla[]="\033[1;30m";  // Black  Text, Bold
char    BRed[]="\033[1;31m";  // Red    Text, Bold
char    BGre[]="\033[1;32m";  // Green  Text, Bold
char    BYel[]="\033[1;33m";  // Yellow Text, Bold
char    BBlu[]="\033[1;34m";  // Blue   Text, Bold
char    BPur[]="\033[1;35m";  // Purple Text, Bold
char    BCya[]="\033[1;36m";  // Cyan   Text, Bold
char    BWhi[]="\033[1;37m";  // White  Text, Bold
char    UBla[]="\033[4;30m";  // Black  Text, Underlined
char    URed[]="\033[4;31m";  // Red    Text, Underlined
char    UGre[]="\033[4;32m";  // Green  Text, Underlined
char    UYel[]="\033[4;33m";  // Yellow Text, Underlined
char    UBlu[]="\033[4;34m";  // Blue   Text, Underlined
char    UPur[]="\033[4;35m";  // Purple Text, Underlined
char    UCya[]="\033[4;36m";  // Cyan   Text, Underlined
char    UWhi[]="\033[4;37m";  // White  Text, Underlined
char    IBla[]="\033[0;90m";  // Black  Text, High Intensity
char    IRed[]="\033[0;91m";  // Red    Text, High Intensity
char    IGre[]="\033[0;92m";  // Green  Text, High Intensity
char    IYel[]="\033[0;93m";  // Yellow Text, High Intensity
char    IBlu[]="\033[0;94m";  // Blue   Text, High Intensity
char    IPur[]="\033[0;95m";  // Purple Text, High Intensity
char    ICya[]="\033[0;96m";  // Cyan   Text, High Intensity
char    IWhi[]="\033[0;97m";  // White  Text, High Intensity
char   BIBla[]="\033[1;90m";  // Black  Text, Bold High Intensity
char   BIRed[]="\033[1;91m";  // Red    Text, Bold High Intensity
char   BIGre[]="\033[1;92m";  // Green  Text, Bold High Intensity
char   BIYel[]="\033[1;93m";  // Yellow Text, Bold High Intensity
char   BIBlu[]="\033[1;94m";  // Blue   Text, Bold High Intensity
char   BIPur[]="\033[1;95m";  // Purple Text, Bold High Intensity
char   BICya[]="\033[1;96m";  // Cyan   Text, Bold High Intensity
char   BIWhi[]="\033[1;97m";  // White  Text, Bold High Intensity
char  On_Bla[]="\033[40m";    // Black  Background
char  On_Red[]="\033[41m";    // Red    Background
char  On_Gre[]="\033[42m";    // Green  Background
char  On_Yel[]="\033[43m";    // Yellow Background
char  On_Blu[]="\033[44m";    // Blue   Background
char  On_Pur[]="\033[45m";    // Purple Background
char  On_Cya[]="\033[46m";    // Cyan   Background
char  On_Whi[]="\033[47m";    // White  Background
char On_IBla[]="\033[0;100m"; // Black  High Intensity Background
char On_IRed[]="\033[0;101m"; // Red    High Intensity Background
char On_IGre[]="\033[0;102m"; // Green  High Intensity Background
char On_IYel[]="\033[0;103m"; // Yellow High Intensity Background
char On_IBlu[]="\033[0;104m"; // Blue   High Intensity Background
char On_IPur[]="\033[0;105m"; // Purple High Intensity Background
char On_ICya[]="\033[0;106m"; // Cyan   High Intensity Background
char On_IWhi[]="\033[0;107m"; // White  High Intensity Background

#endif //_ERROR_COLORS_H

/*# http://stackoverflow.com/questions/16843382/colored-shell-script-output-library
RCol='\e[0m'    # Text Reset

# Regular           Bold                Underline           High Intensity      BoldHigh Intens     Background          High Intensity Backgrounds
Bla='\e[0;30m';     BBla='\e[1;30m';    UBla='\e[4;30m';    IBla='\e[0;90m';    BIBla='\e[1;90m';   On_Bla='\e[40m';    On_IBla='\e[0;100m';
Red='\e[0;31m';     BRed='\e[1;31m';    URed='\e[4;31m';    IRed='\e[0;91m';    BIRed='\e[1;91m';   On_Red='\e[41m';    On_IRed='\e[0;101m';
Gre='\e[0;32m';     BGre='\e[1;32m';    UGre='\e[4;32m';    IGre='\e[0;92m';    BIGre='\e[1;92m';   On_Gre='\e[42m';    On_IGre='\e[0;102m';
Yel='\e[0;33m';     BYel='\e[1;33m';    UYel='\e[4;33m';    IYel='\e[0;93m';    BIYel='\e[1;93m';   On_Yel='\e[43m';    On_IYel='\e[0;103m';
Blu='\e[0;34m';     BBlu='\e[1;34m';    UBlu='\e[4;34m';    IBlu='\e[0;94m';    BIBlu='\e[1;94m';   On_Blu='\e[44m';    On_IBlu='\e[0;104m';
Pur='\e[0;35m';     BPur='\e[1;35m';    UPur='\e[4;35m';    IPur='\e[0;95m';    BIPur='\e[1;95m';   On_Pur='\e[45m';    On_IPur='\e[0;105m';
Cya='\e[0;36m';     BCya='\e[1;36m';    UCya='\e[4;36m';    ICya='\e[0;96m';    BICya='\e[1;96m';   On_Cya='\e[46m';    On_ICya='\e[0;106m';
Whi='\e[0;37m';     BWhi='\e[1;37m';    UWhi='\e[4;37m';    IWhi='\e[0;97m';    BIWhi='\e[1;97m';   On_Whi='\e[47m';    On_IWhi='\e[0;107m';

#Coding:
# https://developer.apple.com/library/mac/documentation/opensource/conceptual/shellscripting/advancedtechniques/advancedtechniques.html
# http://ascii-table.com/ansi-escape-sequences.php
# http://www.cplusplus.com/forum/unices/36461/
# Esc[Value;...;Valuem
# Escape characters:
# C++: '^[' or '['? '\033[' works
# Bash (echo -e): '\e' or '\033'
# Value:
# Tens position: 
#     0x: Control codes
#     3x: Foreground (text) coloring
#     4x: Background coloring
#
# Ones position:
#  Control codes (0x):
#     0: All attributes off  
#     1: Bold on  
#     4: Underscore (on monochrome display adapter only)  
#     5: Blink on  
#     7: Reverse video on  
#     8: Concealed on  
#  Color Codes (3x or 4x):
#     0: Black  
#     1: Red  
#     2: Green  
#     3: Yellow  
#     4: Blue  
#     5: Magenta  
#     6: Cyan  
#     7: White  
#
# echo with variables: echo -e "${Red}"
*/