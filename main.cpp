#include <iostream>
#include <mysql.h>
#include<mysqld_error.h>
#include<windows.h>
#include<sstream>
using namespace std;

const char* HOST ="localhost";
const char* USER = "root";
const char* PW = "root";
const char* DB = "mydb";

class Seats{
private:
	int Seat[5][10];
public:
Seats(){
	for(int i=0;i<5;i++){
		for(int j=0;j<10;j++){
			Seat[i][j]=1;
		}
	}
}
int getSeatStatus(int row,int seatNumber){
	if(row<1 || row>5 || seatNumber<1 || seatNumber>10){
		return -1;
	}
	return Seat[row-1][seatNumber-1];
}
void reserveSeat(int row, int seatNumber){
	if(row<1 || row>5 || seatNumber<1 || seatNumber>10){
		return ;
	}
	//valid hai so 1 se 0 kardo, now it's reserved
	Seat[row-1][seatNumber-1]=0;
}
void display(){
	cout<<" ";
	for(int i=0;i<10;i++){
		cout<<" "<<i+1;
	}
	cout<<endl;
	for(int row=0;row<5;row++){
		cout<<row+1<<" ";
		for(int col=0;col<10;col++){
			//1 means empty and 0 means reserved
			if(Seat[row][col]==1){
				cout<<"- ";
			}
			else{
				cout<<"X ";
			}
		}
		cout<<"|"<<endl;
	}
	cout<<"-----------------------"<<endl;
	
}
void getDB(MYSQL* conn){
	string query ="SELECT RowNumber,SeatNumber,Seat FROM Ticket";
	if(mysql_query(conn,query.c_str())){
		cout<<"Error: "<<mysql_error(conn)<<endl;
		return;
	}
	//structure to get result from db
	MYSQL_RES* result;
	result=mysql_store_result(conn);
	//if result doesn't exists then throw error
	if(!result){
		cout<<"Error: "<<mysql_error(conn)<<endl;
		return;
	}
	//fetching row from result
	MYSQL_ROW row;
	while((row=mysql_fetch_row(result))){
		int rowNumber= atoi(row[0]);
		int seatNumber= atoi(row[1]);
		int seatStatus=atoi(row[2]);
		//db se ye values milne ke baad we are updating our array "seat" with those value
		Seat[rowNumber-1][seatNumber-1]=seatStatus;
	}
	//result use karne ke baad free bhi karna hai
	mysql_free_result(result);
}
};

int main() {
	//declaring object of the constructor
	Seats s;
	//this is to make connection with sql
MYSQL* conn;
	conn=mysql_init(NULL);
	if(!mysql_real_connect(conn,HOST,USER,PW,DB,3306,NULL,0)){
		cout<<"Error: "<<mysql_error(conn)<<endl;
		return 1;
	}	
	else{
		cout<<"Logged In Database!"<<endl;
	}
	Sleep(3000);//this will make error or logged in to display for 3 seconds
	
	//creating a table and if there is an error it is cout
	if(mysql_query(conn,"CREATE TABLE IF NOT EXISTS Ticket(RowNumber INT,SeatNumber INT, Seat INT)")){
		cout<<"Error: "<<mysql_error(conn)<<endl;	
	}
	//inserting data into db
	for(int row=1; row<=5; row++){
		for(int seatNumber=1;seatNumber<=10;seatNumber++){
			stringstream ss;
			ss<<"INSERT INTO Ticket(RowNumber, SeatNumber, Seat) "
			<<"SELECT '"<<row<<"','"<<seatNumber<<"', '1'  "
			// inserting data and checking to avoid duplicacy
			<<"WHERE NOT EXISTS(SELECT * FROM Ticket WHERE RowNumber='"<<row<<"' AND SeatNumber= '"<<seatNumber<<"' )";	
			//ss ko string me convert kiya then insert in db
			string insertQuery=ss.str();
			//checking for error
			if(mysql_query(conn,insertQuery.c_str())){
				cout<<"Error: "<<mysql_error(conn);
			}
		}		
	}
	Sleep(3000);
	
	bool exit=false;
	while(!exit){
		//previous output removed and current output displayed
		system("cls");
		cout<<endl;	
		cout<<"Welcome to Movie Ticket Reservation System"<<endl;
		cout<<"******************************************"<<endl;
		cout<<"1. Reserve A Ticket"<<endl;
		cout<<"2. Exit"<<endl;
		cout<<"Enter Your Choice: ";
		int val;
		cin>>val;

	if(val==1){
	   s.getDB(conn);
	   s.display();
	   
	   int row,col;
	   cout<<"Enter Row(1-5): ";
	   cin>>row;
	   cout<<"Enter Seat Number(1-10): ";
	   cin>>col;
	   //handling invalid input 
	   if(row<1 || row>5 || col<1 || col>10){
			cout<<"Invalid Row or Seat Number!"<<endl;
			Sleep(3000);
			continue;
	   }
	   //getting seat status for given input
	   int seatStatus= s.getSeatStatus(row,col);
	   if(seatStatus==-1){
	    	cout<<"Invalid Row or Seat Number!"<<endl;
	    	Sleep(3000);
	    	continue;
		}
		if(seatStatus==0){
			cout<<"Sorry: Seat is already reserved!"<<endl;
			Sleep(3000);
			continue;
		}
		s.reserveSeat(row,col);
		//c++ me to reserve kardia ab db me update karenge
		stringstream ss;
		ss<<"UPDATE Ticket SET Seat=0 WHERE RowNumber="<<row<<" AND SeatNumber="<<col; 
		//update ki jo query usme koi error to nahi hai uske liye ye update variable 
		//bana kar check karenge
		string update=ss.str();
		if(mysql_query(conn,update.c_str())){
			cout<<"Error: "<<mysql_error(conn)<<endl;
		}
		else{
			cout<<"Seat is Reserved Successfully in Row "<<row<<" and Seat Number "<< col << endl;
		}
		Sleep(5000);
	}//if val==1 ends here
	else if(val==2){
		exit=true;
		cout<<"Good Bye!"<<endl;
		Sleep(3000);
	}
	//1 ya 2 ki jagah koi invalid input hua to
	else{
		cout<<"Invalid Input"<<endl;
		Sleep(3000);
	}
	
	
	}//while loop ends here
	mysql_close(conn);	
	
	return 0;
}
