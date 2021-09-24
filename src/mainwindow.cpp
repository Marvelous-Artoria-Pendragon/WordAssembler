#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "HashTable.h"
#include <string.h>
#include <fstream>
#include <QTextCodec>
#include <QFileDialog>
#include <QMessageBox>
#define GAP 200
#define LINEMAX 1000
using namespace std;

string cpp_path = "";
QTextCodec *code = QTextCodec::codecForName("GB2312");
string appdir = "";

int Hash(char *key)  //计算哈希值
{
    int cnt = 0, len = strlen(key);
    for (int i = 0; i < len && key[i] != '\0'; i++)
        cnt += int(key[i]);
    return cnt;
}

HashTable<string, int> *createDict(string path)     //创建Hash字典
{
    HashTable<string, int> *dict = new HashTable<string, int>(17, 100);
    ifstream infile(path, ios::in);
    if (!infile)                // 文件打开错误处理
        {QMessageBox::critical(NULL, "错误", "字典创建失败!");}

    while (infile)
    {
        char reserved_word[100] = {0};
        infile.getline(reserved_word, 100);
        int key = Hash(reserved_word);
        string temp(reserved_word);
        dict->insert(key, temp);           // 关键字插入
    } infile.close(); return dict;
}

bool isNum(char a)   // 判断一个字符是否为数字
{return ('0' <= a && a <= '9') ? true : false;}

bool isLetter(char a)   // 判断一个字符是否为字母
{return (('a' <= a && a <= 'z') || ('A' <= a && a <= 'Z')) ? true : false;}

bool createBranch()         //创建单词分支判别数组
{
    int *arr = new int[GAP + 256];
    for (int i = 0; i < GAP + 256; i++) arr[i] = 0;
    //标识符为1：下划线、字母开头
    arr[GAP + '_'] = 1;
    for (int i = 0; i < 26; i++)
        arr[GAP + i + 65] = arr[GAP + i + 97] = 1;
    //常量为2：数字开头
    for (int i = 0 ; i < 10; i++) arr[GAP + 48 + i] = 2;
    //运算符为5：特殊字符开头
    string symbol_path = appdir + "/special symbol.txt";
    //char symbol_path[100] = "special symbol.txt";
    ifstream infile(symbol_path, ios::in);
    if (!infile)
        {QMessageBox::critical(NULL, "错误", "特殊符号文件打开错误!"); return false;}
    char symbol[6] = {0};
    while (infile.getline(symbol, 6))
    {
        arr[GAP + symbol[0]] = 5;
        memset(symbol, 0, sizeof(symbol));
    } infile.close();
    //单引号、双引号为3：''，"字符串
    arr[GAP + '\''] = arr[GAP + '\"'] = 3;
    //斜杠为4：/*，// 注释
    arr[GAP + '/'] = 4;
    //分界符为6：, () [] {} ;
    arr[GAP + ','] = arr[GAP + '('] = arr[GAP + ')'] = arr[GAP + '['] =
    arr[GAP + ']'] = arr[GAP + '{'] = arr[GAP + '}'] = arr[GAP + ';'] = 6;

    // //井号为7：#，预处理器关键字的宏定义
    arr[GAP + '#'] = 7;
    //其他字符为0：不包括中文的其他字符
    string save_path = appdir + "/branch.dat";
    //char save_path[50] = "WordAssembler/branch.dat";
    ofstream outfile(save_path, ios::out|ios::binary);
    outfile.write((char *)arr, sizeof(arr[0]) * (GAP + 256));
    outfile.close(); delete[] arr; return true;
}

bool complie(QTextBrowser *browser)          //编译
{
    browser->clear();
    //分支数组读取
    //char branch_path[100] = "branch.dat";
    string branch_path = appdir + "/branch.dat"; int branch[GAP + 256] = {0};
    ifstream branchfile(branch_path, ios::in|ios::binary);
    if (!branchfile)
    {
        createBranch(); // 文件缺失错误处理
        branchfile.open(branch_path, ios::in|ios::binary);
        if (!branchfile) {QMessageBox::critical(NULL, "错误", "分支数组文件打开失败!"); return false;} // 文件打开错误
    }
    branchfile.read((char *)branch, sizeof(branch[0]) * (GAP + 256));
    branchfile.close();
    // 创建关键字字典
    //char reserved_word_path[100] = "reserved word.txt";
    string reserved_word_path = appdir + "/reserved word.txt";
    HashTable<string, int> *word_dict = createDict(reserved_word_path);

    // 创建特殊字符字典
    //char symbol_path[100] = "special symbol.txt";
    string symbol_path = appdir + "/special symbol.txt";
    HashTable<string, int> *symbol_dict = createDict(symbol_path);

    ifstream cppfile(cpp_path, ios::in);
    if (!cppfile)                   // 文件打开错误处理
        {QMessageBox::critical(NULL, "错误", "解析文件打开错误!"); return false;}
    // 预处理器关键字词典
    string prepp_dict[] = {"elif", "endif", "error", "define", "if", "ifdef", "ifndef", "include", "line", "pragma","undef"};

//    // 结果保存
//    char output_path[50] = "Word Assembler/result.txt";
//    ofstream outfile(output_path, ios::out);
//    if (!outfile)                   //文件打开错误处理
//        {QMessageBox::critical(NULL, "错误", "解析结果保存文件打开错误!"); return false;}
    char line[LINEMAX] = {0};                                // 每次读一行
    bool prepp = false;                                   // 预处理器指令标识
    while (cppfile.getline(line, LINEMAX))                   //单词拼装
    {
        char word[100] = {0}; int j = 0;                 //拼装的单词
        for (int i = 0; i < LINEMAX && line[i] != '\0';)
        {
            if (line[i] == ' ') {i++; continue;}            // 空格，直接跳过
            word[j++] = line[i++];                                      // 拼接、识别出的单词
            switch (branch[GAP + line[i - 1]])          // 切分单词
            {
                case 1:                 // 下划线、字母开头
                {
                    for (; i < LINEMAX && (branch[GAP + line[i]] == 1 || branch[GAP + line[i]] == 2);)
                        word[j++] = line[i++];                  // 拼装
                    browser->insertPlainText(QString::fromLocal8Bit(word) + " ");
                    string value(word);
                    if (word_dict->search(Hash(word), value))
                        browser->insertPlainText("关键字\n");
                    else
                        browser->insertPlainText("标识符\n");
                    memset(word, 0, sizeof(word)); j = 0; break;       // 重置word数组
                }
                case 2:                 // 数字开头，包括带后缀的整数、浮点数、指数
                {
                    if (word[j - 1] == '0')
                    {
                        if (line[i] == 'x' || line[i] == 'X')             // 十六进制
                        {
                            word[j++] = line[i++];
                            for (; i < LINEMAX && ((branch[GAP + line[i]] == 2) || ('a' <= line[i] && line[i] < 'g') || ('A' <= line[i] && line[i] < 'G'));)
                                word[j++] = line[i++];
                            if (('f' < line[i] && line[i] <= 'z') || ('F' < line[i] && line[i] <= 'Z'))       // 十六进制结尾不能为g~z字母
                                {QMessageBox::critical(NULL, "错误", "十六进制数格式错误!"); return false;}
                        } else if (line[i] == '.')                                                      // 0.xxx浮点数
                        {
                            if (line[i] == 'e' || line[i] == 'E')                                      // 浮点数的指数形式表示
                            {
                                word[j++] = line[i++];
                                if (line[i] == '-') word[j++] = line[i++];                                  // 幂部分的负号跳过
                                for (; i < LINEMAX && branch[GAP + line[i]] == 2;)                             // 拼装指数部分
                                    word[j++] = line[i++];
                                if (line[i] == 'l' || line[i] == 'L') {word[j++] = line[i++]; break;}       // 高精度浮点数
                            }
                        } else if (isLetter(line[i]) || line[i] == '_')
                                {QMessageBox::critical(NULL, "错误", "八进制数格式错误!"); return false;}                // 第二位非x，数字，不合法
                        else
                        {
                            for (; i < LINEMAX && ('0' <= line[i] && line[i] < '8');)                          // 八进制
                                word[j++] = line[i++];
                            if (('8' <= line[i] && line[i] <= '9') || branch[GAP + line[i]] == 1)       // 八进制数末尾不能为8、9，及字母、下划线
                                {QMessageBox::critical(NULL, "错误", "八进制数格式错误!"); return false;}
                        }
                    } else                              // 十进制数
                    {
                        for (; i < LINEMAX && branch[GAP + line[i]] == 2;)                             // 拼装整数部分
                            word[j++] = line[i++];
                        if (line[i] == 'u' || line[i] == 'U')                                       // 无符号整数
                        {
                            word[j++] = line[i++];
                            if (line[i] == 'l' || line[i] == 'L') {word[j++] = line[i++];}              // 无符号高整数
                        } else if (line[i] == 'l' || line[i] == 'L') {word[j++] = line[i++];}           // 高精度整数
                        else if (line[i] == 'e' || line[i] == 'E')                                      // 浮点数的指数形式表示
                        {
                            word[j++] = line[i++];
                            if (line[i] == '-') word[j++] = line[i++];                                  // 幂部分的负号跳过
                            for (; i < LINEMAX && branch[GAP + line[i]] == 2;)                             // 拼装指数部分
                                word[j++] = line[i++];
                            if (line[i] == 'l' || line[i] == 'L') {word[j++] = line[i++];}              // 高精度浮点数
                        } else if (line[i] == '.')                                                      // 浮点数
                        {
                            if (!isNum(line[i + 1])) {QMessageBox::critical(NULL, "错误", "小数点后缺乏数字!"); return false;}
                            word[j++] = line[i++];
                            for (; i < LINEMAX && branch[GAP + line[i]] == 2;)                             // 拼装小数部分
                                word[j++] = line[i++];
                            if (line[i] == 'e' || line[i] == 'E')
                            {
                                word[j++] = line[i++];
                                if (line[i] == '-') word[j++] = line[i++];              // 幂部分的负号跳过
                                for (; i < LINEMAX && branch[GAP + line[i]] == 2;)         // 拼装指数部分
                                    word[j++] = line[i++];
                                if (line[i] == 'l' || line[i] == 'L' || line[i] == 'f' || line[i] == 'F')       // 高精度/单精度浮点数指数形式
                                    word[j++] = line[i++];
                            } else if (line[i] == 'l' || line[i] == 'L' || line[i] == 'f' || line[i] == 'F')    // 高精度/单精度浮点数
                                    word[j++] = line[i++];
                        } else if (isLetter(line[i + 1]) || line[i + 1] == '_') {QMessageBox::critical(NULL, "错误", "浮点数格式错误!"); return false;}
                    }
                    browser->insertPlainText(QString::fromLocal8Bit(word) + " 常量\n");
                    memset(word, 0, sizeof(word)); j = 0; break;
                }
                case 3:             // 单引号、双引号开头，串；  不检查单引号内容是否为空
                {
                    char ch = line[i - 1];
                    while (i < LINEMAX && line[i] != '\0')
                    {
                        if (line[i] == '\\') word[j++] = line[i++];
                        else if (line[i] == ch) break;
                        word[j++] = line[i++];
                    }
                    if (line[i] != ch) {QMessageBox::critical(NULL, "错误", "缺乏配对的引号!"); return false;}
                    word[j++] = line[i++];                                  // 拼装右引号
                    browser->insertPlainText(QString::fromLocal8Bit(word) + " 串\n");
                    memset(word, 0, sizeof(word)); j = 0; break;
                }
                case 4:             // 斜杠开头，/*,// 注释；/=,/ 运算符
                {
                    if (line[i] == '*')
                    {
                        word[j++] = line[i++];
                        while (i < LINEMAX && !(line[i] == '/' && line[i - 1] == '*'))     // 未匹配右斜杠
                        {
                            if (line[i] == '\0')
                            {
                                browser->insertPlainText(QString::fromLocal8Bit(word));
                                memset(word, 0, sizeof(word)); j = 0; i = 0;
                                if (!cppfile.getline(line, LINEMAX)) {QMessageBox::critical(NULL, "错误", "缺乏配对的注释符!"); return false;}
                            } word[j++] = line[i++];
                        }
                        browser->insertPlainText(QString::fromLocal8Bit(word) + "/ 注释\n"); break;
                    } else if (line[i] == '/')
                    {
                        while (i < LINEMAX && line[i] != '\0') word[j++] = line[i++];
                        browser->insertPlainText(QString::fromLocal8Bit(word) + " 注释\n");
                        memset(word, 0, sizeof(word)); j = 0; break;
                    } else continue;        // 斜杠为运算符
                }
                case 5:                 // 特殊字符开头
                {
                    int k = 0;
                    for (; i < LINEMAX && k < 2 && branch[GAP + line[i]] == 5;)
                        word[j++] = line[i++];
                    string value = word;
                    if (prepp && value == "##") ;
                    else while (!symbol_dict->search(Hash(word), value) && k > 0)
                        {word[j--] = '\0'; value = word; i--;}     // 不匹配，倒退
                    browser->insertPlainText(QString::fromLocal8Bit(word) + " 运算符\n");
                    memset(word, 0, sizeof(word)); j = 0; break;
                }
                case 6:         // 分界符：, () [] {} ;
                {
                    browser->insertPlainText(QString::fromLocal8Bit(word) + " 分界符\n");
                    memset(word, 0, sizeof(word)); j = 0; break;
                }
                case 7:             // 井号，预处理器关键字：
                {
                    browser->insertPlainText("# 特殊符号\n");
                    prepp = true; j--;
                    for (; i < LINEMAX && (branch[GAP + line[i]] == 1);)        // 井号后只能为只含字母的关键字
                        word[j++] = line[i++];                  // 拼装
                    string value(word); int k = 0;
                    for (; k < 11; k++)
                        if (prepp_dict[k] == value)
                        {
                            browser->insertPlainText(QString::fromLocal8Bit(word) + " 预处理器关键字\n");
                            memset(word, 0, sizeof(word)); j = 0;
                            if (value == "include")
                            {
                                while (i < LINEMAX && line[i] == ' ') i++;
                                if (line[i] == '<')
                                {
                                    word[j++] = line[i++];
                                    for (; i < LINEMAX && line[i] != '\0' && line[i] != '>' && line[i] != ' ';)
                                        word[j++] = line[i++];
                                    if (line[i] != '>') {QMessageBox::critical(NULL, "错误", "缺乏配对的括号或包含路径格式错误!"); return false;}
                                    browser->insertPlainText(QString::fromLocal8Bit(word) + "> 头文件\n"); i++;
                                    memset(word, 0, sizeof(word)); j = 0;
                                } else if (line[i] == '\"') i--;
                            } break;
                        }
                    if (k == 11) {QMessageBox::critical(NULL, "错误", "未识别的预处理器关键字!"); return false;} break;
                }
                default:                // 语法错误
                {QMessageBox::critical(NULL, "错误", "语法错误!"); return false;}
            }
        } memset(line, 0, sizeof(line));
    } cppfile.close();
    return true;
}


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    appdir = code->fromUnicode(QApplication::applicationDirPath()).data();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_opencpp_triggered()
{
    QTextCodec *code = QTextCodec::codecForName("GBK");
    cpp_path = code->fromUnicode(QFileDialog::getOpenFileName(NULL, "打开文件", ".")).data();
    if (cpp_path == "") return;
    else ui->textBrowser_1->clear();
    ifstream infile(cpp_path, ios::in);
    char line[LINEMAX] = {0};
    while (infile.getline(line, LINEMAX))
        ui->textBrowser_1->insertPlainText(QString::fromLocal8Bit(line) + "\n");
}

void MainWindow::on_parse_triggered()
{
    if (cpp_path == "") {QMessageBox::warning(NULL, "警告", "请先选择解析文件!"); return;}
    if (complie(ui->textBrowser_2)) QMessageBox::information(NULL, "提示", "解析成功!");
    else QMessageBox::critical(NULL, "错误", "解析失败！");
}
