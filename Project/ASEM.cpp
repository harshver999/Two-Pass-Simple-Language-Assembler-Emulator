#include<bits/stdc++.h>
using namespace std;

unordered_map<string, int> mnemonics;               //mnemonic, opcode
unordered_map<int, string> err;                     //error code, error message
unordered_map<int, string> warn;                    //warning code, warning message

vector<string> prog_lines;                          //lines of the program
vector<pair<int,int>> errors;                       //line number, error code
vector<pair<int,int>> warnings;                     //line number, warning code
map<string, int> symbol_table;                      //label, program counter(address)
vector<pair<int, string>> forward_references;       //line number, label

string basefile;

void load();
void loadMnemonics();
void loadErrors();
void loadWarnings();
void commentsRemover(string &s);
void trim(string &s);
string basef(string s);
bool isLabel(string s);
bool operandNeeded(string s);
bool isValidNumber(string s);
bool isValidhex(string s);
int hextoDec(string s);
string intToHex(int n, int width=8){
    stringstream ss;
    ss<<hex<<n;
    string res=ss.str();
    while(res.size()<width){
        res="0"+res;
    }
    while(res.size()>width){
        res.pop_back();
    }
    return res;
}
bool isOffset(string s);
bool isValue(string s);
bool sortbysec(const pair<int,int> &a, const pair<int,int> &b){
    return (a.second < b.second);
}

void first_pass(){
    int size=prog_lines.size();
    int pc=0;   //program counter
    int i=0;
    set<string> declared_labels;
    map<string, int> used_labels;
    map<string, vector<int>> reqlabels; //label, line numbers
    bool HALT=false;
    while(i!=size){
        string curr_line=prog_lines[i];
        istringstream ss(curr_line);
        string token;
        ss>>token;
        string next_token="";
        int pos=token.find_first_of(':');
        if(pos!=string::npos){
            next_token=token;
            next_token=next_token.substr(pos+1);
            token=token.substr(0, pos+1);
        }
        bool labelbefore=false;
        if(token.back()==':'){
            labelbefore=true;
            token.pop_back();
            if(!isLabel(token)){
                errors.push_back({pc, 7});
            }
            if(declared_labels.find(token)!=declared_labels.end()){
                errors.push_back({pc, 1});
            }
            //cout<<token<<endl;
            symbol_table[token]=pc;
            declared_labels.insert(token);
            if(next_token==""){
                if(!(ss>>token)){
                    i++;
                    continue;
                }
            }
            else{
                token=next_token;
            }
        }
        string mnem=token;
        if(mnemonics.find(token)==mnemonics.end() || (mnem=="SET" && (!labelbefore))){
            //cout<<mnem<<endl;
            errors.push_back({pc, 8});
            i++;     pc++;
            continue;
        }
        if(token=="HALT"){
            HALT=true;
        }
        vector<string> operands;
        while(ss>>token){
            operands.push_back(token);
        }
        if(!operandNeeded(mnem) && !operands.empty()){
            errors.push_back({pc, 5});
            i++;     pc++;
            continue;
        }
        if(!operandNeeded(mnem) && operands.empty()){
            pc++;
            i++;
            continue;
        }
        if(operandNeeded(mnem) && operands.empty()){
            errors.push_back({pc, 4});
            i++;     pc++;
            continue;
        }
        if(operands.size()>1){
            errors.push_back({pc, 6});
            i++;    pc++;
            continue;
        }
        string operandd=operands[0];
        if(mnem=="SET"){
            if(isValidNumber(operandd)){
                symbol_table[token]=stoi(operandd);
            }
            else if(isValidhex(operandd)){
                symbol_table[token]=hextoDec(operandd);
            }
            else{
                errors.push_back({pc, 3});
            }
            i++;     pc++;    
            continue;
        }
        // cout<<mnem<<" ";
        // for(auto &it:operands){
        //     cout<<it<<" ";
        // }cout<<endl;
        if(!(isValidhex(operandd) || isValidNumber(operandd))){
            if(!((operandd[0]>='a' && operandd[0]<='z') || (operandd[0]>='A' && operandd[0]<='Z') || operandd[0]=='_')){
                errors.push_back({pc, 3});
                i++;    pc++;
                continue;
            }
            else{
                reqlabels[operandd].push_back(pc);
                //cout<<operandd<<" "<<pc<<endl;
            }
            i++;     pc++;
            continue;
        }      
        i++;    pc++;
    }

    if(!HALT){
        warnings.push_back({-1, 3});
    }
    for(auto &it:reqlabels){
        if(symbol_table.find(it.first)==symbol_table.end()){
            for(auto &it2:it.second){
                errors.push_back({it2, 2});
                //cout<<it2<<" "<<it.first<<endl;
            }
        }
    }
    for(auto &it:symbol_table){
        if(reqlabels.find(it.first)==reqlabels.end()){
            for(auto &it2:reqlabels[it.first]){
                warnings.push_back({it2, 1});
            }
        }
    }
    sort(errors.begin(), errors.end(), sortbysec);
    sort(warnings.begin(), warnings.end(), sortbysec);
}

void second_pass(){
    ofstream listingfile(basefile+"lst");
    ofstream binfile(basefile+"o");
    int size=prog_lines.size();
    int pc=0;   //program counter
    int i=0;
    while(i<size){
        string curr_line=prog_lines[i];
        if(curr_line.empty()){
            i++;
            continue;
        }
        istringstream ss(curr_line);
        string token;
        ss>>token;
        string next_token="";
        int pos=token.find_first_of(':');
        if(pos!=string::npos){
            next_token=token;
            next_token=next_token.substr(pos+1);
            token=token.substr(0, pos+1);
        }
        if(token.back()==':'){
            token.pop_back();
            if(next_token==""){
                if(!(ss>>token)){
                    listingfile<<intToHex(pc, 8)<<"\t\t"<<curr_line<<"\n";
                    i++;
                    continue;
                }
            }
            else{
                token=next_token;
            }
        }
        string mnem=token;
        vector<string> operands;
        while(ss>>token){
            operands.push_back(token);
        }
        string mnemm=intToHex(mnemonics[mnem], 2);
        string operandd="000000";
        if(operands.empty() || (!(operandNeeded(mnem)))){
            string res=operandd+mnemm;
            binfile<<res<<"\n";
            listingfile<<intToHex(pc, 8)<<"\t\t"<<res<<"\t\t"<<curr_line<<"\n";
            pc++;
            i++;
            continue;
        }
        string operand=operands[0];
        if(isValue(mnem)){
            if(isValidNumber(operand)){
                operandd=intToHex(stoi(operand), 6);
            }
            else if(isValidhex(operand)){
                operandd=operand.substr(2);
                if(operandd.size()<6){
                    while(operandd.size()<6){
                        operandd="0"+operandd;
                    }
                }
                else{
                    while(operandd.size()>6){
                        operandd.erase(operandd.begin());
                    }
                }
            }
            else{
                operandd=intToHex(symbol_table[operand], 6);
            }
        }
        else if(isOffset(mnem)){
            if(isValidNumber(operand)){
                operandd=intToHex(stoi(operand)-pc, 6);
            }
            else if(isValidhex(operand)){
                operandd=operand.substr(2);
                unsigned int val=hextoDec(operand);
                val-=pc;
                operandd=intToHex(val, 6);
            }
            else{
                operandd=intToHex(symbol_table[operand]-pc, 6);
            }
        }
        string res=operandd+mnemm;
        binfile<<res<<"\n";
        listingfile<<intToHex(pc, 8)<<"\t\t"<<res<<"\t\t"<<curr_line<<"\n";
        pc++;   i++;
    }
}

signed main(int argc, char* argv[]){
    if(argc<2){
        cout<<"Please enter a name.asm file to be compiled.\n";
        return 0;
    }
    string filename=argv[1];
    if(filename.size()<4){
        cout<<"ERROR: The specified file is not of .asm type.\n";
        return 1;
    }
    else if(filename.size()>=4 && filename.substr(filename.size()-4)!=".asm"){
        cout<<"ERROR: The specified file is not of .asm type.\n";
        return 1;
    }
    ifstream file(filename);
    if(!file.is_open()){
        cout<<"ERROR: Could not open the file "<<filename<<"\n";
        return 1;
    }
    load();

    string line;
    while(getline(file, line)){
        commentsRemover(line);
        trim(line);
        if(line.size()){
            prog_lines.push_back(line);
            //cout<<line<<"\n";
        }
    }

    basefile=basef(filename);

    ofstream logfile(basefile+"log");
    // ofstream listingfile(basefile+"l");
    // ofstream binfile(basefile+"o");

    //cout<<basefile<<"\n";

    first_pass();
    // cout<<"Symbol Table:\n";
    // cout<<errors.size()<<"\n";
    if(errors.empty()){
        second_pass();
    }
    else{
        cout<<"Errors found during assembly:\n";
        logfile<<"Errors found during assembly:\n";
        for(auto &it:errors){
            cout<<"Error at line "<<it.first<<": "<<err[it.second]<<"\n";
            logfile<<"Error at line "<<it.first<<": "<<err[it.second]<<"\n";
        }
    }
    for(auto &it:warnings){
        if(it.second==3){
            cout<<"Warning: "<<warn[it.second]<<"\n";
            logfile<<"Warning: "<<warn[it.second]<<"\n";
            continue;
        }
        cout<<"Warning at line "<<it.first<<": "<<warn[it.second]<<"\n";
        logfile<<"Warning at line "<<it.first<<": "<<warn[it.second]<<"\n";
    }
    // for(auto &it:symbol_table){
    //     cout<<it.first<<" -> "<<intToHex(it.second, 8)<<" "<<it.second<<"\n";
    // }
}

void commentsRemover(string &s){
    string res="";
    for(int i=0; i<s.size(); i++){
        if(s[i]==';'){
            break;
        }
        res+=s[i];
    }
    s=res;
}

void trim(string &s){
    while((s.size())){
        if(s.back()==' ' || s.back()=='\t' || s.back()=='\r' || s.back()=='\n'){
            s.pop_back();
        }
        else{
            break;
        }
    }
    reverse(s.begin(), s.end());
    while((s.size())){
        if(s.back()==' ' || s.back()=='\t' || s.back()=='\r' || s.back()=='\n'){
            s.pop_back();
        }
        else{
            break;
        }
    }
    reverse(s.begin(), s.end());
}

void loadMnemonics(){
    mnemonics["data"]=-1;
    mnemonics["ldc"]=0;
    mnemonics["adc"]=1;
    mnemonics["ldl"]=2;
    mnemonics["stl"]=3;
    mnemonics["ldnl"]=4;
    mnemonics["stnl"]=5;
    mnemonics["add"]=6;
    mnemonics["sub"]=7;
    mnemonics["shl"]=8;
    mnemonics["shr"]=9;
    mnemonics["adj"]=10;
    mnemonics["a2sp"]=11;
    mnemonics["sp2a"]=12;
    mnemonics["call"]=13;
    mnemonics["return"]=14;
    mnemonics["brz"]=15;
    mnemonics["brlz"]=16;
    mnemonics["br"]=17;
    mnemonics["HALT"]=18;
    mnemonics["SET"]=19;
}

void loadErrors(){
    err[1]="DUPLICATE LABEL";
    err[2]="NO SUCH LABEL FOUND";
    err[3]="INAVLID OPERAND(NAN)";
    err[4]="MISSING OPERAND";
    err[5]="UNEXPECTED OPERAND";
    err[6]="EXTRA OPERAND";
    err[7]="INVALID LABEL NAME";
    err[8]="INVALID MNEMONIC";
}

void loadWarnings(){
    warn[1]="UNUSED LABEL";
    warn[2]="INFINITE LOOP";
    warn[3]="HALT INSTRUCTION NOT FOUND";
}

void load(){
    loadMnemonics();
    loadErrors();
    loadWarnings();
}

string basef(string s){
    while(s.size()){
        if(s.back()=='.'){
            break;
        }
        s.pop_back();
    }
    return s;
}

bool isLabel(string s){
    if(s.empty()){
        return false;
    }
    if(s[0]>='0' && s[0]<='9'){
        return false;
    }
    int sz=s.size();    
    for(int i=0; i<sz; i++){
        if((s[i]>='a' && s[i]<='z') || (s[i]>='A' && s[i]<='Z') || (s[i]>='0' && s[i]<='9') || s[i]=='_'){
            continue;
        }
        else{
            return false;
        }
    }
    return true;
}

bool operandNeeded(string s){
    if(s=="add" || s=="sub" || s=="shl" || s=="shr" || s=="a2sp" || s=="sp2a" || s=="return" || s=="HALT"){
        return false;
    }
    return true;
}

bool isValidNumber(string s){
    if(s.empty()){
        return false;
    }
    if(s[0]=='-'){
        s.erase(s.begin());
    }
    for(int i=0; i<s.size(); i++){
        if(s[i]>='0' && s[i]<='9'){
            continue;
        }
        return false;
    }
    return true;
}

int hextoDec(string s){
    int res=0;
    int base=1;
    int sz=s.size();
    for(int i=sz-1; i>=0; i--){
        if(s[i]>='0' && s[i]<='9'){
            res+=(s[i]-'0')*base;
        }
        else if(s[i]>='A' && s[i]<='F'){
            res+=(s[i]-'A'+10)*base;
        }
        else if(s[i]>='a' && s[i]<='f'){
            res+=(s[i]-'a'+10)*base;
        }
        base*=16;
    }
    return res;
}

bool isValidhex(string s){
    if(s.empty()){
        return false;
    }
    if(s.size()>2 && s[0]=='0' && (s[1]=='x')){
        s.erase(s.begin()); s.erase(s.begin());
    }
    for(int i=0; i<s.size(); i++){
        if((s[i]>='0' && s[i]<='9') || (s[i]>='A' && s[i]<='F') || (s[i]>='a' && s[i]<='f')){
            continue;
        }
        return false;
    }
    return true;
}

bool isOffset(string s){
    if(s=="call" || s=="brz" || s=="brlz" || s=="br"){
        return true;
    }
    return false;
}

bool isValue(string s){
    if(s=="data" || s=="ldc" || s=="adc" || s=="adj" || s=="SET" || s=="ldl" || s=="ldnl" || s=="stl" || s=="stnl"){
        return true;
    }
    return false;
}