#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <random>
#include <cstdint>
#include <cstdio>
#include "text.hpp"
using namespace std;
bool skip=0;
bool skipToApply=0;
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
int pageNow=0;
void upDown(bool x,string ans) {/* 0 Up    1 Dn*/
    pageNow+=(int)x*2-1;
    if (pageNow<0) {pageNow=0;}
    if (pageNow>22) {pageNow=22;}
    printf("\033[2J\033[u%s\033[H",ans.c_str());

    string ts="Form FORMS-EN-2873-FORM - Page "+to_string(21)+"\n\n";
    int ln=qs[21].c.size()-1;
    if(pageNow>0) {typeString(ts+qs[21].q+"\n\n",1);}
    else {typeString(ts+qs[21].q+"\n\n",15);}
    for (int i=1;i<=15;i++) {
        for (int j=1;j<=3;j++) {
            int num=(j-1)*15+i+104*pageNow;/*correct 45 but [sic]*/
            if (num>ln) {break;}
            // output options
            for (int k=1;k<=(int(log10(ln)))-(int(log10(num)));k++) {printf("0");}
            printf("%d] %s",num,qs[21].c[num].c_str());
            if (j!=3) {
                for (int k=1;k<=120/3-(int(log10(ln))+2+qs[21].c[num].length());k++) {printf(" ");}
            }
        }
        printf("\n");
    }
    printf(texts[20].c_str());
    printf(ans.c_str());
    fflush(stdout);
}
string getLine(bool enableAskUpAndDown=0) {
    char g;
    string ans;
    while (1) {
        g=myGetch();
        if (enableAskUpAndDown) {
#ifdef _WIN32
            if (g==0||224) {
                char g2=myGetch();
                if (g2==73) {upDown(0,ans);continue;} else {ans+=g2;}// PGUP
                if (g2==81) {upDown(1,ans);continue;} else {ans+=g2;}// PGDN
            }
#else
            if (g==27) {
                char g2=myGetch();
                if (g2=='[') {
                    char g3=myGetch();
                    if (g3=='5') {// PGUP
                        char g4=myGetch();
                        if (g4=='~') {upDown(0,ans);continue;} else {ans+=g2;ans+=g3;ans+=g4;}}
                    if (g3=='6') {// PGDN
                        char g4=myGetch();
                        if (g4=='~') {upDown(1,ans);continue;} else {ans+=g2;ans+=g3;ans+=g4;}}
                }
            }
#endif
        }
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
int main(int argc,char* argv[]){
    for(int i=1;i<argc;i++) {
        string arg=argv[i];
        if (arg=="-skip") {skip=1;}
    }
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    keepRunning=1;
    string uid=getLocalUid();
    stateEnum state=UNLOGIN;
    string username,password;
    rawMode(1);
    initQ();
    initTexts();
    printf("\033[?1049h");
    printf("\033[2J\033[H");
    fflush(stdout);
    char g;
    bool passwordWrong=0;
    bool bossKeyMode=0;
    int step=-1;
    string header="GLaDOS v1.07 (c) 1982 Aperture Science, Inc.",message="",prompt="B:\\>";
    string ts;
    while(keepRunning) {
        input="";
        switch (state) {
            case UNLOGIN:
                printf("\033[2J\033[H");//clr
                goto_UNLOGIN_2:;
                typeString("> ",0);
                input=getLine();
                fflush(stdout);
                if (input=="LOGIN") {state=USERNAME;}
                else if (input=="HELP") {printf("\033[2J\033[H");typeString(texts[19],10);goto goto_UNLOGIN_2;}
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
                password="";
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
                    if (password=="TIER3") {header="GLaDOS v1.07a (c) 1982 Aperture Science, Inc.";prompt="ADMIN> ";passwordWrong=0;state=LOGINED;}
                    else {passwordWrong=1;}
                } else {
                    if (password=="PORTAL"||password=="PORTALS") {passwordWrong=0;state=LOGINED;}
                    else {passwordWrong=1;}
                }
                break;
            case LOGINED:
                printf("\033[2J\033[H");
                typeString(header+message+"\n\n"+prompt,7);
                input=getLine();
                if (input=="THECAKEISALIE") {state=CAKE;}
                else if (input=="DIR"||input=="CATALOG"||input=="DIRECTORY"||input=="LIST"||input=="LS"||input=="CAT") {if (username=="CJOHNSON") {message=texts[0];} else {message=texts[1];}}
                else if (input=="IP") {message=" \n\nuid:"+uid+"\n";}
                else if (input=="HELP"||input=="LIB"||input=="?") {if(username=="CJOHNSON") {message=texts[2];} else {message=texts[3];}}
                else if (input=="LOGOUT"||input=="BYE"||input=="LOGOFF"||input=="VALUE") {/*open steam*/;state=UNLOGIN;}
                else if (input=="APPEND"||input=="ATTRIB"||input=="COPY"||input=="FORMAT"||input=="ERASE"||input=="RENAME") {message=texts[4];}
                else if (input=="PLAY") {message=texts[5];}
                else if (input=="PLAY PORTAL") {/*PORTAL!*/;state=LOGINED;}
                else if (input=="INTERROGATE") {message=texts[6];}
                else if (input[0]=='I'&&input[1]=='N'&&input[2]=='T'&&input[3]=='E'&&input[4]=='R'&&input[5]=='R'&&input[6]=='O'&&input[7]=='G'&&input[8]=='A'&&input[9]=='T'&&input[10]=='E') {if (username=="CJOHNSON") {message=texts[7];} else {message=texts[8];}}
                else if (input=="TAPEDISK") {message=texts[9];}
                else if (input=="NOTES"||input=="NOTES.EXE") {if (username=="CJOHNSON") {state=NOTES;}else {message="\n\nERROR 24 [File \'"+input+"\' not found]";}}
                else if (input=="APPLY"||input=="APPLY.EXE") {step=-1;state=APPLY;}
                else {message="\n\nERROR 24 [File \'"+input+"\' not found]";}
                break;
            case CAKE:
                if (!bossKeyMode) {
                    printf("\033[2J\033[H");
                    printf(texts[10].c_str());
                    g=0;
                    while(1){
                        g=myGetch();
                        if(g) {
                            bossKeyMode=1;
                            break;
                        }
                    }
                } else {
                    printf("\033[2J\033[H");
                    printf(texts[11].c_str());
                    fflush(stdout);
                    while(1){
                        g=myGetch();
                        if(g) {
                            bossKeyMode=0;
                            break;
                        }
                    }
                }
                break;
            case NOTES:
                printf("\033[2J\033[H");typeString(texts[12],3);
                g=0;while(1){g=myGetch();if(g) {
                    printf("\033[2J\033[H");typeString(texts[13],3);
                    g=0;while(1){g=myGetch();if(g) {
                        printf("\033[2J\033[H");typeString(texts[14],3);
                        g=0;while(1){g=myGetch();if(g) {
                            printf("\033[2J\033[H");typeString(texts[15],3);
                            g=0;while(1){g=myGetch();if(g) {
                                state=LOGINED;
                            break;}}
                        break;}}
                    break;}}
                break;}}
                break;
            case APPLY:
                printf("\033[2J\033[H");
                if (step==-1) {
                    typeString(texts[16],5);
                    goto_1:;
                    input=getLine();
                    if (input=="QUIT") {state=LOGINED;}
                    if (input=="CONTINUE") {step++;}
                    else {printf("\033[2K\033[G> ");fflush(stdout);goto goto_1;}
                } else if (step==0) {
                    typeString(texts[17],10);
                    printf("[%s]", uid.c_str());fflush(stdout);
                    typeString(texts[18],10);
                    printf("\033[2J\033[H");fflush(stdout);
                    printf(texts[17].c_str());
                    printf("\033[5m[%s]\033[0m", uid.c_str());fflush(stdout);
                    printf(texts[18].c_str());
                    fflush(stdout);
                    goto0:;
                    input=getLine();
                    if (input=="QUIT") {state=LOGINED;}
                    if (input=="CONTINUE") {step++;}
                    else {printf("\033[2K\033[G> ");fflush(stdout);goto goto0;}
                } else if (step==51){
                    typeString(texts[21],10);
                    input=getLine();
                    printf("\033[2J\033[H");
                    typeString(texts[22],10);
                    while (1) {printf("\033[2K\033[G");fflush(stdout);getLine();}
                } else {
                    ts="Form FORMS-EN-2873-FORM - Page "+to_string(step)+"\n\n";
                    if (qs[step].t=='T') {
                        typeString(ts+qs[step].q+"\n\n> ",25);
                        input=getLine(); if (input=="QUIT") {state=LOGINED;}
                        step++;
                    }
                    else {
                        int ln=qs[step].c.size()-1;
                        int col=4;
                        if (ln>150||ln<=48) {col=3;}
                        if (ln<=19) {col=1;}
                        if (step==3) {
                            typeString(ts+qs[step].q+"\n\n",15);
                            for (int i=1;i<=16;i++) {
                                for (int j=1;j<=2;j++) {
                                    int num=(j-1)*16+i;
                                    if (num>ln) {break;}
                                    // output options
                                    for (int k=1;k<=(int(log10(ln)))-(int(log10(num)));k++) {printf("0");}
                                    printf("%d] %s",num,qs[step].c[num].c_str());
                                    if (j!=2) {
                                        for (int k=1;k<=120/3-(int(log10(ln))+2+qs[step].c[num].length());k++) {printf(" ");}
                                    }
                                }
                                printf("\n");
                            }
                            printf("\n");
                            gotoBecauseInputIsllegal_1:;
                            printf("\r\033[2K> ");
                            fflush(stdout);
                            input=getLine();
                            if (input=="QUIT") {state=LOGINED;}
                            for (int i=0;i<input.size();i++) {if (!(input[i]==' '||(input[i]>='0'&&input[i]<='9'))) {goto gotoBecauseInputIsllegal_1;}}
                            int l=input.find_first_not_of(' ');
                            int r=input.find_last_not_of(' ');
                            if (l==string::npos) {goto gotoBecauseInputIsllegal_1;}
                            input=input.substr(l,r-l+1);
                            int p=input.find_first_not_of('0');
                            if (p==string::npos) {goto gotoBecauseInputIsllegal_1;}
                            input=input.substr(p);
                            for (int i=0;i<input.size();i++) {if (input[i]==' ') {goto gotoBecauseInputIsllegal_1;}}
                            if (stoi(input)>ln) {goto gotoBecauseInputIsllegal_1;}
                            step++;
                        } else if (step==21) {
                            if(pageNow>0) {typeString(ts+qs[step].q+"\n\n",1);}
                            else {typeString(ts+qs[step].q+"\n\n",15);}
                            for (int i=1;i<=15;i++) {
                                for (int j=1;j<=3;j++) {
                                    int num=(j-1)*15+i;
                                    if (num>ln) {break;}
                                    // output options
                                    for (int k=1;k<=(int(log10(ln)))-(int(log10(num)));k++) {printf("0");}
                                    printf("%d] %s",num,qs[step].c[num].c_str());
                                    if (j!=3) {
                                        for (int k=1;k<=120/3-(int(log10(ln))+2+qs[step].c[num].length());k++) {printf(" ");}
                                    }
                                }
                                printf("\n");
                            }
                            printf("\n");
                            gotoBecauseInputIsllegal_2:;
                            printf("\r\033[2K");
                            printf(texts[20].c_str());
                            printf("\033[s");
                            fflush(stdout);
                            input=getLine(1);
                            if (input=="QUIT") {state=LOGINED;}
                            for (int i=0;i<input.size();i++) {if (!(input[i]==' '||(input[i]>='0'&&input[i]<='9'))) {goto gotoBecauseInputIsllegal_2;}}
                            int l=input.find_first_not_of(' ');
                            int r=input.find_last_not_of(' ');
                            if (l==string::npos) {goto gotoBecauseInputIsllegal_2;}
                            input=input.substr(l,r-l+1);
                            int p=input.find_first_not_of('0');
                            if (p==string::npos) {goto gotoBecauseInputIsllegal_2;}
                            input=input.substr(p);
                            for (int i=0;i<input.size();i++) {if (input[i]==' ') {goto gotoBecauseInputIsllegal_2;}}
                            if (stoi(input)>ln) {goto gotoBecauseInputIsllegal_2;}
                            step++;
                        } else {
                            typeString(ts+qs[step].q+"\n\n",15);
                            if (col==1) {
                                for (int i=1;i<=ln;i++) {
                                    int num=i;
                                    // output options
                                    for (int k=1;k<=(int(log10(ln)))-(int(log10(num)));k++) {printf("0");}
                                    printf("%d] %s",num,qs[step].c[num].c_str());
                                    printf("\n");
                                }
                            } else {
                                for (int i=1;i<=19;i++) {
                                    for (int j=1;j<=col;j++) {
                                        int num=(j-1)*19+i;
                                        if (num>ln) {break;}
                                        // output options
                                        for (int k=1;k<=(int(log10(ln)))-(int(log10(num)));k++) {printf("0");}
                                        printf("%d] %s",num,qs[step].c[num].c_str());
                                        if (j!=col) {
                                            for (int k=1;k<=120/col-(int(log10(ln))+2+qs[step].c[num].length());k++) {printf(" ");}
                                        }
                                    }
                                    printf("\n");
                                }
                            }
                            printf("\n");
                            gotoBecauseInputIsllegal_3:;
                            printf("\r\033[2K> ");
                            fflush(stdout);
                            input=getLine();
                            if (input=="QUIT") {state=LOGINED;}
                            for (int i=0;i<input.size();i++) {if (!(input[i]==' '||(input[i]>='0'&&input[i]<='9'))) {goto gotoBecauseInputIsllegal_3;}}
                            int l=input.find_first_not_of(' ');
                            int r=input.find_last_not_of(' ');
                            if (l==string::npos) {goto gotoBecauseInputIsllegal_3;}
                            input=input.substr(l,r-l+1);
                            int p=input.find_first_not_of('0');
                            if (p==string::npos) {goto gotoBecauseInputIsllegal_3;}
                            input=input.substr(p);
                            for (int i=0;i<input.size();i++) {if (input[i]==' ') {goto gotoBecauseInputIsllegal_3;}}
                            if (stoi(input)>ln) {goto gotoBecauseInputIsllegal_3;}
                            step++;
                        }
                    }
                }
                break;
        }
    }
}