#include <iostream>
#include <fstream>
#include <stack>
#include <cstdio>
#include <cstring>
using namespace std;
//堆栈
stack<int> state_stack;
stack<char> sign_stack;

char G[300][300];     //存文法
int length[300];    //文法的长度
int number=0;    //文法的个数
bool isV[300];      //buffer of input
char Vn[300];        //非终结符
int size_vn=0;      
char Vt[300];       //终结符
int size_vt=0;
bool first[300][300];    //first集
char buffer[300];           
int size=0;
struct T        //转换表  项目集转换
{
		int begin;
		int next;
		char ch;
};
struct project      //项目集
{
		int num;
		int now;
		char search;
};
struct action
{
		char ch;
		int next_state;
};


T trans[300];
int size_trans=0;
project items[300][300];
int count=0;
int size_item[300];

action action_table[300][300];
int size_act_table[300];

//文件定义
ifstream grammar_file;
ifstream input_file;
ofstream items_file;
ofstream action_file;
ofstream firstset_file;
ofstream procedure_file;

void get_grammar()     //读文法
{
		char nouse,temp;
		int i,j=0;
		G[0][0]='S';    //S‘->`S
		length[0]=2; 
		grammar_file>>number;
		for(i=1;i<=number;++i)
		{
				j=0;
				grammar_file>>temp;
				grammar_file>>nouse>>nouse;
				while(temp!='$')
				{
						G[i][j++]=temp;
						isV[temp]=true;
						grammar_file>>temp;
				}
				length[i]=j; 
		}
		G[0][1]=G[1][0];
	 //ASCII码表
		for(i=0;i<64;++i)
		{
				if(isV[i])
				{
					 	Vt[size_vt++]=i;
				}
		}
		for(i=65;i<91;++i)
		{
				if(isV[i])
				{
					 	Vn[size_vn++]=i;
				}
		}
		for(i=91;i<128;++i)
		{
				if(isV[i])
				{
					 	Vt[size_vt++]=i;
				}
		}
}
bool Is_in_vn(char a)
{
		for(int i=0;i<size_vn;++i)
		{
				if(Vn[i]==a)
				{
						return true;
				}
		}
		return false;
}  
void get_first()
{
		bool done=true;    //还有能改的地方 就不停的做
		int t,k;
		bool isempty;
		while(done)
		{
				done=false;
				for(int i=1;i<=number;i++)
				{
					 	t=1;
					 	isempty=true;
					 	while(isempty && t<length[i])
					 	{
							 	isempty=false;
							 	if(G[i][t]>='A' && G[i][t]<='Z')
							 	{
										for(k=0;k<=63;++k)
										{
											if(first[G[i][t]-'A'][k]==true && !first[G[i][0]-'A'][k])
											{
													first[G[i][0]-'A'][k]=true;
													done=true;
											}
										}
										if(first[G[i][t]-'A'][64]==true)    //用@表示 空
										{
												isempty=true;
												++t;
										}
										for(k=91;k<128;++k)
										{
											if(first[G[i][t]-'A'][k]==true && first[G[i][0]-'A'][k]==false)
											{
													done=true;
													first[G[i][0]-'A'][k]=true;	
											}
										}		
								}
								else if(first[G[i][0]-'A'][G[i][t]]==false)
								{
									done=true;
									first[G[i][0]-'A'][G[i][t] ]=true;	
								}
					 	}
					 	if(length[i]==t)
					 	{
								first[G[i][0]-'A'][26]=true;
					 	}
				}
		}
}
void write_first_set()
{
		for(int i=0;i<26;++i)
		{
				char ch=char(i+'A');
				if(Is_in_vn(ch))
				{
						/*printf("first(%c)=",ch);
						for(int j=0;j<128;++j)
						{
								if(first[i][j]==true)
								{
										cout<<char(j)<<",";
								}
						}
						cout<<endl;*/
						firstset_file<<"first("<<ch<<")={";
						for(int j=0;j<128;++j)
						{
								if(first[i][j]==true)
								{
										firstset_file<<char(j)<<",";
								}
						}
						firstset_file<<"}"<<endl;
				}
		}
}

 
void gete_search(project temp)    //得到向前搜索符
{
		size=0;
		bool flag=true;
		int nownow=temp.now;
		int i;
		while(flag==true)
		{
				flag=false;
				if(nownow+1>=length[temp.num])
				{
						buffer[size++]=temp.search;
						return;
				}
				else if(G[temp.num][nownow+1]<'A' || G[temp.num][nownow+1]>'Z')
				{
								buffer[size++]=G[temp.num][nownow+1];
								return;
				}   
				else if(G[temp.num][nownow+1]>='A' && G[temp.num][nownow+1]<='Z')
				{
						for(i=0;i<64;++i)
						{
								if(first[G[temp.num][nownow+1]-'A'][i]==true)
										buffer[size++]=i;
						}
						for(i=91;i<128;++i)
						{
								if(first[G[temp.num][nownow+1]-'A'][i]==true)
								{
										buffer[size++] = i;
								}
						}
						if(first[G[temp.num][nownow+1]-'A'][64]==true)
						{
								++nownow;
								flag=true;
						}
				}  
		}
}

bool is_in(project temp,int T)   //当前项目集元素是否重复
{
		int i;
		for(i=0;i<size_item[T];++i)
		{
				if(items[T][i].num==temp.num && items[T][i].now==temp.now && items[T][i].search==temp.search)
				{
						return true;
				}
		}
		return false;
}
 
void e_closure(int T)   //求项目集族
{
		project temp;
		int i,j,k;
		for(i=0;i<size_item[T];++i)
		{
			 	if(G[items[T][i].num][items[T][i].now]>='A' && G[items[T][i].num][items[T][i].now]<='Z')
			 	{
					 	for(j=0;j<300;++j)
					 	{
					 		size=0; 
					 		if(G[j][0]==G[items[T][i].num][items[T][i].now])
							{
								 		gete_search(items[T][i]);
								 		for(k=0;k<size;++k)
								 		{
												temp.num=j;
												temp.now=1;
												temp.search=buffer[k];
												if(is_in(temp,T)==false)
												{
													 	items[T][size_item[T]++]=temp;
												}
								 		}	
							}
					 	}	 	
				}
		}
		return;
}
 
int is_contained()   //当前项目集 和以前比较
{
		int i;
		int sum=0;
		int j;
		int k;
		for(i=0;i<count;++i)
		{
			 	sum=0;        //记录有多少是匹配的
			 	if(size_item[count]==size_item[i])
			 	{
			 		for(j=0;j<size_item[count];++j)
					{
							for(k=0;k<size_item[i];++k)
							{
								if(items[i][k].num==items[count][j].num && items[i][k].now==items[count][j].now && items[i][k].search==items[count][j].search)
								{
										++sum;
										break;
								}
							}		
					}
			 	}		
			 	if(sum==size_item[count])
			 	{
						return i;
			 	}
		}
		return 0;
}
 
void make_set()
{
		items[0][0].num=0;
		items[0][0].now=1;
		items[0][0].search='#';
		size_item[0]=1;
		e_closure(0);
		project buf[50];
		int buf_size=0;
		project tp;
		int i,j,k;
		int nextt_state;
		items_file<<"I0:"<<endl;
		for(i=0;i<size_item[0];++i)
		{
				items_file<<G[items[0][i].num][0]<<"->"<<G[items[0][i].num]+1<<" , "<<items[0][i].now<<" , "<<items[0][i].search<<endl;
		}
		items_file<<"--------------------------------------------------"<<endl;
	 	int index;
	 	int p;
	 	int t;
		for(index=0;index<count+1;++index)
		{
				for(j=0;j<size_vt;++j)
				{
						buf_size=0;
						for(p=0;p<size_item[index];++p)
						{
								if(items[index][p].now<length[items[index][p].num] && G[items[index][p].num][items[index][p].now]==Vt[j])
								{
										tp.num=items[index][p].num;
										tp.search=items[index][p].search;
										tp.now=items[index][p].now+1;
										buf[buf_size++]=tp;
								}
						}
						if(buf_size!= 0)    //产生一个新的项目集
						{ 
								count++;
								for(t=0;t<buf_size;++t)
								{
										items[count][size_item[count]++]=buf[t];
								}
								e_closure(count);
								nextt_state=is_contained();        //检查第count个项目集是否重复
								if(nextt_state!=0)           //重复，则转换到已有的那个项目集中去
								{
										size_item[count--]=0;
										trans[size_trans].begin=index;
										trans[size_trans].next=nextt_state;
										trans[size_trans++].ch=Vt[j];
								}
								else
								{
										items_file<<"I"<<count<<":"<<endl;
										for(i=0;i<size_item[count];++i)
										{
												items_file<<G[items[count][i].num][0]<<"->"<<G[items[count][i].num]+1<<" , "<<items[count][i].now<<" , "<<items[count][i].search<<endl;
										}
										items_file<<"--------------------------------------------------"<<endl;
										trans[size_trans].begin=index;
										trans[size_trans].next=count;
										trans[size_trans++].ch=Vt[j];     
								}
						}
				}
 
				for(j=0;j<size_vn;++j)
				{
						buf_size = 0;
						for(p=0;p<size_item[index];++p)
						{
								if(items[index][p].now<length[items[index][p].num] && G[items[index][p].num][items[index][p].now]==Vn[j])
								{
										tp.num=items[index][p].num;
										tp.search=items[index][p].search;
										tp.now=items[index][p].now+1;
										buf[buf_size++]=tp;
								}
						}
						if(buf_size!=0)
						{
								++count;
								for(t=0;t<buf_size;++t)
								{
										items[count][size_item[count]++]=buf[t];
								}
								e_closure(count);
								nextt_state=is_contained();
									
								if(nextt_state!=0)
								{
										size_item[count--]=0;
										trans[size_trans].begin=index;
										trans[size_trans].next=nextt_state;
										trans[size_trans++].ch=Vn[j];
								}
								else
								{
										items_file<<"I"<<count<<":"<<endl;
										for(i=0;i<size_item[count];++i)
										{
												items_file<<G[items[count][i].num][0]<<"->"<<G[items[count][i].num]+1<<" , "<<items[count][i].now<<" , "<<items[count][i].search<<endl;
										}
										items_file<<"--------------------------------------------------"<<endl;
										trans[size_trans].begin=index;
										trans[size_trans].next=count;
										trans[size_trans++].ch=Vn[j];
								}                  
						}
				}
		}
}

void get_action()
{
		int i,j;
		int t1,t2,t;
		char tp;
		for(i=0;i<300;++i)
		{
				size_act_table[i]=0;
		}
		for(i=0;i<=count;++i)
		{
				for(j=0;j<size_item[i];++j)
				{
					if(items[i][j].now==length[items[i][j].num] || (items[i][j].now==1 && length[items[i][j].num]==2 && G[items[i][j].num][1]=='@'))
						{
								action_table[i][size_act_table[i]].ch=items[i][j].search;
								action_table[i][size_act_table[i]++].next_state=items[i][j].num*(-1);
						}
				}	
		}
		for(i=0;i<size_trans;++i)
		{
				t1=trans[i].begin;
				t2=trans[i].next;
				tp=trans[i].ch;
				action_table[t1][size_act_table[t1]].ch=tp;
				action_table[t1][size_act_table[t1]++].next_state=t2;
		}
		for(i = 0; i <= count; ++i)
    	{
	      for(j = 0; j < size_act_table[i]; ++j) 
	      {
	            tp=action_table[i][j].ch;
	            t=action_table[i][j].next_state;
	            action_file << "(" << i << "," << tp << "," << t << ")" << endl;
	      }
    }
}
void write_stack(int x)
{
	stack<int> a;
	stack<char> c;
	if(x==1)    //状态
	{
		while(state_stack.empty()==false)
		{
			a.push(state_stack.top());
			state_stack.pop();
		}
		while(a.empty()==false)
		{
			procedure_file<<a.top()<<',';
			state_stack.push(a.top());
			a.pop();
		}
	}
	if(x==2)   //符号
	{
		while(sign_stack.empty()==false)
		{
			c.push(sign_stack.top());
			sign_stack.pop();
		}
		while(c.empty()==false)
		{
			procedure_file<<c.top()<<',';
			sign_stack.push(c.top());
			c.pop();
		}
	}
}
void judge()
{
	size = 0;
	while(input_file>>buffer[size] && buffer[size++] != '#');
    int    work_sta = 0;
    int    index_buf = 0;
    bool   error=false;
    bool   done = false;
    char now_in;
    now_in=buffer[0];

    state_stack.push(0);
    sign_stack.push('#');
    procedure_file<<"状态栈                 符号栈                  输入串               ACTION                  GOTO"<<endl;
    int i,j,k,m;
    int tp,len;
    int aa;
    while(done == false && error == false)
    {
        work_sta = state_stack.top();
        write_stack(1);
        procedure_file<<"               ";
        write_stack(2);
        procedure_file<<"               ";
        for(i=index_buf;i<size;++i)
        {
        		procedure_file<<buffer[i];
        }
        procedure_file<<"              ";
        error =  true;
        for(i= 0; i < size_act_table[work_sta];++i)
            if(action_table[work_sta][i].ch ==now_in)
            {
                    error = false;
                    if(action_table[work_sta][i].next_state == 0)
                    {
                                    procedure_file << "YES" << endl;
                                    cout<<"YES"<<endl;
                                    done = true;
                                    break;
                    }
                    else if(action_table[work_sta][i].next_state > 0)   //移进
                    {
                    		procedure_file<<'S'<<action_table[work_sta][i].next_state<<endl;

                       	state_stack.push(action_table[work_sta][i].next_state);
                       	sign_stack.push(action_table[work_sta][i].ch);
                        ++index_buf;
                        now_in=buffer[index_buf];
                        break;
                    }
                    else if(action_table[work_sta][i].next_state < 0)   
                    {
                       	tp = action_table[work_sta][i].next_state*(-1);    
                       	procedure_file<<'r'<<tp<<"              ";
                       	len=length[tp]-1;
                       	if(G[tp][1]=='@')
                       	{
                       			--len;
                       	}
                       	for(k = 0; k < len; ++k)
                       	{
                       		state_stack.pop();
                       		sign_stack.pop();
                       	}
                        sign_stack.push(G[tp][0]);
                        aa=state_stack.top();
                        for(m=0;m<size_act_table[aa];++m)
                        {
                        	if(action_table[aa][m].ch==G[tp][0])
                        	{
                        		state_stack.push(action_table[aa][m].next_state);
                        		procedure_file<<action_table[aa][m].next_state<<endl;
                        	}
                        }
                        break;
                    }
            }
    }
    if(!done)
    {
        cout << "NO" << endl;
        cout<<"出错原因可能是未找到：";
        for(i=0;i<size_act_table[work_sta];++i)
        {
        		cout<<action_table[work_sta][i].ch<<" ";
        }
        cout<<endl;
    }
}
void init()
{
		int i,j;
		for(i=0;i<300;++i)
		{
				isV[i]=false;
		}
		for(i=0;i<300;++i)
		{
				size_item[i]=0;
		}
		for(i=0;i<300;++i)
		{
				for(j=0;j<300;++j)
				{
						first[i][j]=false;
				}
		}
		size=0;
}
void open_file()
{
	grammar_file.open("F:\\编译原理课程设计答辩\\语法分析_文法.txt");
    input_file.open("F:\\编译原理课程设计答辩\\output.txt");
    items_file.open("F:\\编译原理课程设计答辩\\项目集.txt");
    action_file.open("F:\\编译原理课程设计答辩\\分析表.txt");
    firstset_file.open("F:\\编译原理课程设计答辩\\first集.txt");
    procedure_file.open("F:\\编译原理课程设计答辩\\分析过程.txt");
}
void LR1()
{
	get_grammar();
    get_first();
    write_first_set();
    make_set();
    get_action();
    judge(); 
}
void close_file()
{
	grammar_file.close();
    input_file.close();
    items_file.close();
    action_file.close();
    firstset_file.close();
    procedure_file.close();
}
int main()
{
    init();   
    open_file();
    LR1();
    close_file();
    return 0;
}

