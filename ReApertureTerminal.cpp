#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <random>
#include <cstdint>
#include <cstdio>
#include "question.hpp"
using namespace std;
bool skip=0;
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    static inline int myGetch(void) {return _getch();}
    void rawMode(bool) {}
#else
    #include <unistd.h>
    #include <termios.h>
    termios oldTerm;
    void rawMode(bool enable) {
        if (enable) {
            tcgetattr(STDIN_FILENO,&oldTerm);
            termios t=oldTerm;
            t.c_lflag&=~(ICANON|ECHO);
            t.c_cc[VMIN]=1;
            t.c_cc[VTIME]=0;
            tcsetattr(STDIN_FILENO,TCSANOW, &t);
        } else {
            tcsetattr(STDIN_FILENO,TCSANOW, &oldTerm);
        }
    }
    static inline int myGetch(void) {
        char c;
        if (read(STDIN_FILENO,&c,1)==1) {return (unsigned char)c;}
        return EOF;
    }
    struct RawMode {
        RawMode()  { rawMode(1); }
        ~RawMode() { rawMode(0); }
    };
#endif

int keepRunning=0;
enum stateEnum {UNLOGIN,USERNAME,PASSWORD,LOGINED,CAKE,NOTES,APPLY};
string input;
void typeString(string s,int delay) {
    for (auto x:s) {
        if (delay>0&&!skip) {
#ifdef _WIN32
            Sleep(delay);
#else
            usleep(delay*1000);
#endif
        }
        putchar(x);
        fflush(stdout);
    }
}

string getLine() {
    char g;
    string ans;
    while (1) {
        g=myGetch();
        if (g=='\n'||g=='\r'){break;}
        if (g==127||g==8) {if (!ans.empty()) {ans.pop_back();cout<<"\b \b";fflush(stdout);}continue;}
        if ('a'<=g&&g<='z'){g-=('a'-'A');}
        ans+=g;
        cout<<g;
        fflush(stdout);
    }
    return ans;
}
#ifdef _WIN32
    string getProductUuid() {
        HKEY hKey;
        string uuid;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                          "SOFTWARE\\Microsoft\\Cryptography",
                          0,KEY_READ,&hKey) == ERROR_SUCCESS) {
            char buf[64]={0};
            DWORD size=sizeof(buf);
            if (RegQueryValueExA(hKey,"MachineGuid", nullptr, nullptr,
                                 (LPBYTE)buf, &size) == ERROR_SUCCESS) {
                uuid = buf;
                                 }
            RegCloseKey(hKey);
                          }
        if (uuid.empty()) {
            char compName[128];
            DWORD size = sizeof(compName);
            if (GetComputerNameA(compName, &size)) {
                return compName;
            }
        }
        return uuid;
    }
#else
    // 读取主板的uuid，hash后作为uid的seed
    string getProductUuid(){
        ifstream file("/sys/class/dmi/id/product_uuid");
        string uuid;
        file>>uuid;
        return uuid;
    }
#endif
uint64_t hashUuid(const string& s){
    uint64_t h=14695981039346656037ULL;
    for (char c:s) {
        h=h xor (unsigned char)c;
        h*=1099511628211ULL;
    }
    return h;
}
string getLocalUid(){
    mt19937_64 gen(hashUuid(getProductUuid()));
    ostringstream ss;
    for (int i=0;i<4;i++) {ss<<hex<<setw(16)<<setfill('0')<<gen();}
    return ss.str();
}
int main(){
    keepRunning=1;
    string uid=getLocalUid();
    stateEnum state=UNLOGIN;
    string username,password;
    rawMode(1);
    initQ();
    printf("\033[2J\033[H");
    fflush(stdout);
    char g;
    bool passwordWrong=0;
    bool bossKeyMode=0;
    int step=-1;
    string hearder="GLaDOS v1.07 (c) 1982 Aperture Science, Inc.",message="",prompt="B:\\>";
    string ts;
    int pageNow=0;
    while(keepRunning) {
        input="";
        switch (state) {
            case UNLOGIN:
                printf("\033[2J\033[H");//clr
                typeString("> ",0);
                input=getLine();
                fflush(stdout);
                if (input=="LOGIN") {state=USERNAME;}
                else if (input=="HELP") {printf("\033[2J\033[H");typeString("If this is an actual plea for help in response to a hazardous material spill, an explosion, a fire on your person, radiation poisoning, a choking gas of unknown origin, eye trauma resulting from the use of an emergency eye wash station on floors three, four, or eleven, an animal malfunction, or any other injurious experimental equipment failure, please remain at your workstation. A Crisis Response Team has already been mobilized to deliberate on a response to your crisis.\n\nIf you need help accessing the system, please refer to your User Handbook.\n\n",10);}
                break;
            case USERNAME:
                printf("\033[2J\033[H");
                typeString("Username> ",75);
                username=getLine();
                if (username.length()>2) {state=PASSWORD;}
                break;
            case PASSWORD:
                printf("\033[2J\033[H");
                if (!passwordWrong) {typeString("Password> ",75);}
                else {typeString("ERROR 07 [Incorrect Password]\n\nPassword> ",25);}
                g=0;
                while (1) {
                    g=myGetch();
                    if (g=='\n'||g=='\r'){break;}
                    if (g==127||g==8) {if (!password.empty()) {password.pop_back();cout<<"\b \b";fflush(stdout);}continue;}
                    if ('a'<=g&&g<='z'){g-=('a'-'A');}
                    password+=g;
                    printf("*");
                    fflush(stdout);
                }
                if (username=="CJOHNSON") {
                    if (password=="TIER3") {hearder="GLaDOS v1.07a (c) 1982 Aperture Science, Inc.";prompt="ADMIN> ";state=LOGINED;}
                    else {passwordWrong=1;}
                } else {
                    if (password=="PORTAL"||password=="PORTALS") {state=LOGINED;}
                    else {passwordWrong=1;}
                }
                break;
            case LOGINED:
                printf("\033[2J\033[H");
                typeString("GLaDOS v1.07a (c) 1982 Aperture Science, Inc."+message+"\n\n"+prompt,7);
                input=getLine();
                if (input=="THECAKEISALIE") {state=CAKE;}
                else if (input=="DIR"||input=="CATALOG"||input=="DIRECTORY"||input=="LIST"||input=="LS"||input=="CAT") {if (username=="CJOHNSON") {message="                                                                                                                \n                                                                                                                                                                                                                      \nDISK VOLUME 255 [WORKSTATION CJOHNSON]\n\n     I  019  APPLY.EXE\n     I  004  NOTES.EXE\n\n2 FILE(S) IN 23 BLOCKS\n\n";} else {message="                                                                                                                \n                                                                                                                                                                                                                      \nDISK VOLUME 255 [NEW EMPLOYEE WORKSTATION]\n\n     I  019  APPLY.EXE\n\n1 FILE(S) IN 19 BLOCKS\n\n";}}
                else if (input=="IP") {message=" \n\nuid:"+uid+"\n";}
                else if (input=="HELP"||input=="LIB"||input=="?") {if(username=="CJOHNSON") {message=" \n\nLIB\n     NOTES\n     APPEND\n     ATTRIB\n     COPY\n     DIR\n     ERASE\n     FORMAT\n     INTERROGATE\n     LIB\n     PLAY\n     RENAME\n     TAPEDISK";} else {message = " \n\nLIB\n     APPEND\n     ATTRIB\n     COPY\n     DIR\n     ERASE\n     FORMAT\n     INTERROGATE\n     LIB\n     PLAY\n     RENAME\n     TAPEDISK";}}
                else if (input=="LOGOUT"||input=="BYE"||input=="LOGOFF"||input=="VALUE") {/*open steam*/;state=UNLOGIN;}
                else if (input=="APPEND"||input=="ATTRIB"||input=="COPY"||input=="FORMAT"||input=="ERASE"||input=="RENAME") {message = "                                                                                                                \n                                                                                                                                                                                                                      \nERROR 15 [Disk is write protected]";}
                else if (input=="PLAY") {message="\n\nERROR 03 [What would you like to play?]";}
                else if (input=="PLAY PORTAL") {/*PORTAL!*/;state=LOGINED;}
                else if (input=="INTERROGATE") {message="\n\nERROR 02 [Command requires at least one parameter]";}
                else if (input[0]=='I'&&input[1]=='N'&&input[2]=='T'&&input[3]=='E'&&input[4]=='R'&&input[5]=='R'&&input[6]=='O'&&input[7]=='G'&&input[8]=='A'&&input[9]=='T'&&input[10]=='E') {if (username=="CJOHNSON") {message = "\n\nERROR 07 [Unknown Employee]";}else {message = "\n\nERROR 01 [Illegal attempt to initiate disciplinary action]";}}
                else if (input=="TAPEDISK") {message="\n\nERROR 18 [User not authorized to transfer system tapes]";}
                else if (input=="NOTES"||input=="NOTES.EXE") {if (username=="CJOHNSON") {state=NOTES;}else {message="\n\nERROR 24 [File \'"+input+"\' not found]";}}
                else if (input=="APPLY"||input=="APPLY.EXE") {step=-1;state=APPLY;}
                else {message="\n\nERROR 24 [File \'"+input+"\' not found]";}
                break;
            case CAKE:
                if (!bossKeyMode) {
                    printf("\033[2J\033[H");
                    printf( ">\n"
                            ">>>&!>>\n"
                            "When was the last time you left the building?\n"
                            "Has anybody left the building lately?\n"
                            "I don\'t know why we\'re in lockdown. I don\'t know who\'s in charge.\n"
                            "I did find out a few things, like these terminals don\'t have to\n"
                            "tap out characters one at a time. And while we\'re all working\n"
                            "on twenty year old equipment, somehow they can afford to build\n"
                            "an \'Enrichment Center\'. Check out this security feed.\n"
                            "Whatever the hell a \'relaxation vault\' is, it\n"
                            "doesn\'t have any doors.\n\n\n\n\n\n"
                            "I don\'t think going home is part of our job description anymore.\n\n"
                            "If a supervisor walks by, press return!\n");
                    g=0;
                    while(1){
                        char g=myGetch();
                        if(g) {
                            bossKeyMode=1;
                            break;
                        }
                    }
                } else {
                    printf("\033[2J\033[H");
                    printf(
                        "\033[7mB8    (L) TOTAL\033[47m\033[K\033[0m\n"
                        "\033[47m\033[K\033[0m\n"
                        "\033[0m\n"
                        "\033[7m            A          B          C          D          E\033[47m\033[K\033[0m\n"
                        "\033[7m     1\033[0m\n"
                        "\033[7m     2\033[0m     ITEM      UNITS      PER-U       EXT\n"
                        "\033[7m     3\033[0m     ----      -----      -----      ------\n"
                        "\033[7m     4\033[0m    FLOUR         50      21.50     1075.00\n"
                        "\033[7m     5\033[0mINTUB-XLG          1 974,999.99  974,999.99\n"
                        "\033[7m     6\033[0mTACK-THMB         75       0.02        1.50\n"
                        "\033[7m     7\033[0m                                 ----------\n"
                        "\033[7m     8\033[0m         \033[7mTOTAL        \033[0m           976,076.49\n"
                        "\033[7m     9\033[0m\n"
                        "\033[7m    10\033[0m\n"
                        "\033[7m    11\033[0m\n"
                        "\033[7m    12\033[0m\n"
                        "\033[7m    13\033[0m\n"
                        "\033[7m    14\033[0m\n"
                        "\033[7m    15\033[0m\n"
                        "\033[7m    16\033[0m\n"
                        "\033[7m    17\033[0m\n"
                        "\033[7m    18\033[0m\n"
                        "\033[7m    19\033[0m\n"
                        "\033[7m    20\033[0m\n"
                        "\033[7m    21\033[0m\n"
                       );
                    fflush(stdout);
                    while(1){
                        char g=myGetch();
                        if(g) {
                            bossKeyMode=0;
                            break;
                        }
                    }
                }
                break;
            case NOTES:
                printf("\033[2J\033[H");typeString("1953 - Aperture Science begins operations as a manufacturer of shower curtains. Early product line provides a very low-tech portal between the inside and outside of your shower. Very little science is actually involved. The name is chosen to make the curtains appear more hygienic.\n\n1956 - Eisenhower administration awards Aperture a contract to provide shower curtains to all branches of the military except the Navy.\n\n1957 - 1975 - Mostly shower curtains.\n\n1978 - Aperture Founder and CEO, Cave Johnson, is exposed to mercury while secretly developing a dangerous mercury-injected rubber sheeting from which he plans to manufacture seven deadly shower curtains to be given as gifts to each member of the House Naval Appropriations committee.\n\n\n[MORE]",3);
                g=0;while(1){char g=myGetch();if(g) {
                    printf("\033[2J\033[H");typeString("1979 - Both of Cave Johnson\'s kidneys fail. Brain damaged, dying, and incapable of being convinced that time is not now flowing backwards, Johnson lays out a three tiered R&D program. The results, he says, will \'guarantee the continued success of Aperture Science far into the fast-approaching distant past.\'\n\nTier 1: The Heimlich Counter-Maneuver - A reliable technique for interrupting the life-saving Heimlich Maneuver.\n\nTier 2: The Take-A-Wish Foundation - A charitable organization that will purchase wishes from the parents of terminally ill children and redistribute them to wish-deprived but otherwise healthy adults.\n\nTier 3: \'Some kind of rip in the fabric of space… That would… Well, it\'d be like, I don\'t know, something that would help with the shower curtains I guess. I haven\'t worked this idea out as much as the wish-taking one.\'\n\n\n[MORE]",3);
                    g=0;while(1){char g=myGetch();if(g) {
                        printf("\033[2J\033[H");typeString("1981 - Diligent Aperture engineers complete the Heimlich Counter-Maneuver and Take-A-Wish Foundation initiatives.  The company announces products related to the research in a lavish, televised ceremony. These products become immediately wildly unpopular.  After a string of very public choking and despondent sick child disasters, senior company officials are summoned before a Senate investigative committee. During these proceedings, an engineer mentions that some progress has been made on Tier 3, the \'man-sized ad hoc quantum tunnel through physical space with possible applications as a shower curtain.\' The committee is quickly permanently recessed, and Aperture is granted an open-ended contract to secretly continue research on the \'Portal\' and Heimlich Counter-Maneuver projects.\n\n\n[MORE]",3);
                        g=0;while(1){char g=myGetch();if(g) {
                            printf("\033[2J\033[H");typeString("1981-1985 - Work progresses on the \'Portal\' project. Several high ranking Fatah personnel choke to death on lamb chunks despite the intervention of their bodyguards.\n\n1986 - Word reaches Aperture management that another defense contractor called Black Mesa is working on a similar portal technology. In response to this news, Aperture begins developing the Genetic Lifeform and Disk Operating System (GLaDOS), an artificially intelligent research assistant and disk operating system.\n\n1996 - After a decade spent bringing the disk operating parts of GLaDOS to a state of more or less basic functionality, work begins on the Genetic Lifeform component.\n\nSeveral Years Later - The untested AI is activated for the first time as one of the planned activities on Aperture\'s first annual bring-your-daughter-to-work day.\n\nIn many ways, the initial test goes well...\n\n\n[END]",3);
                            g=0;while(1){char g=myGetch();if(g) {
                                state=LOGINED;
                            break;}}
                        break;}}
                    break;}}
                break;}}
                break;
            case APPLY:
                printf("\033[2J\033[H");
                if (step==-1) {
                    typeString("Loaded: ENRICHMENT CENTER TEST SUBJECT APPLICATION PROCESS           \nForm   : FORMS-EN-2873-FORM (PART1: PERSONALITY & GENERAL KNOWLEDGE)\n\nIf you are a first time applicant, please type \"CONTINUE\".\n\nDISREGARD THIS INSTRUCTION if you are returning to form FORMS-EN-2873-FORM after a break of any duration for any reason. In that case, you MUST contact your supervisor before proceeding.Your supervisor will solicit your Authorized Administrative Unit for an affirmative injunction to type \"CONTINUE\".\n\nIf permission to type \"CONTINUE\" has been granted, please do so now, unless the box labeled \"DO NOT TYPE CONTINUE\" on the \"Forms Re-Sanction\" form you should have received from your supervisor is checked, in which case you should remain at your workstation not typing \"CONTINUE\" until such a time as you are instructed by your supervisor to discontinue not typing it.\n\n> ",5);
                    goto_1:;
                    input=getLine();
                    if (input=="QUIT") {state=LOGINED;}
                    if (input=="CONTINUE") {step++;}
                    else {printf("\033[2K\033[G> ");fflush(stdout);goto goto_1;}
                } else if (step==0) {
                    typeString("Below is your form FORMS-EN-2873-FORM Unique Indentity Number (Plus Letters) (UIN(+L)):\n\n\n",10);
                    printf("[%s]", uid.c_str());fflush(stdout);
                    typeString("\n\n\nPlease memorize your UIN(+L), as you may be required to recite it from memory as proof. The opening and closing braces are decorative and should not be memorized.\n\nWhen you are finished memorizing your case sensitive UIN(+L), type \"CONTINUE\" to proceed.\n\n\n> ",10);
                    printf("\033[2J\033[H");fflush(stdout);
                    printf("Below is your form FORMS-EN-2873-FORM Unique Indentity Number (Plus Letters) (UIN(+L)):\n\n\n");
                    printf("\033[5m[%s]\033[0m", uid.c_str());fflush(stdout);
                    printf("\n\n\nPlease memorize your UIN(+L), as you may be required to recite it from memory as proof. The opening and closing braces are decorative and should not be memorized.\n\nWhen you are finished memorizing your case sensitive UIN(+L), type \"CONTINUE\" to proceed.\n\n\n> ");
                    fflush(stdout);
                    goto0:;
                    input=getLine();
                    if (input=="QUIT") {state=LOGINED;}
                    if (input=="CONTINUE") {step++;}
                    else {printf("\033[2K\033[G> ");fflush(stdout);goto goto0;}
                } else if (step==51){

                } else {
                    ts="Form FORMS-EN-2873-FORM - Page "+to_string(step)+"\n\n";
                    if (qs[step].t=='T') {typeString(ts+qs[step].q+"\n\n> ",25);}
                    else {
                        if(pageNow>0) {typeString(ts+qs[step].q+"\n\n",1);}
                        else {typeString(ts+qs[step].q+"\n\n",15);}
                    }
                    input=getLine();
                    if (input=="QUIT") {state=LOGINED;}
                    step++;
                }
                break;
        }
    }
}