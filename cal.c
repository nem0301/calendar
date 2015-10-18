#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

//option 1 Gregory, 0 Julius
int isLeapYear(int year, int opt)
{
	if (opt && year <= 1752)
	{
		if (year%4 == 0)
			return 1;
		else
			return 0;
	}
	else
	{
		if ( (year%4 == 0) && ((year%100 != 0) || (year%400 == 0)) )
			return 1;
		else
			return 0;
	}
}

int getDays(int year, int month, int opt)
{
	switch (month)
	{
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
			return 31;
		case 4: case 6: case 11:
			return  30;
		case 2:
			if ( isLeapYear(year, opt) )
				return 29;
			else
				return 28;
		case 9:
			if( year == 1752)
				return 19;
			else
				return 30;
	}
}

// 0 = sun ... 6 = sat
int getStartWeekDay (int year, int month, int opt)
{
	int i;
	int days;
	if (opt)
		days = 6;
	else
		days = 5;			//(6 + 13)%7 = 5


	//accumulate days before this year
	for (i = 1; i < year; i++)
	{
		if (isLeapYear(i, opt))
		{
			if ( i == 1752 )
				days += 355;
			else
				days += 366;
		}
		else
		{
			days += 365;
		}
	}

	for (i = 1; i < month; i++)
	{
		days += getDays(year, i, opt);
	}

	return days % 7;
}

//input current month-1
int juliusStartDay (int year, int month)
{
	int days = 0;
	if (month == 0)
	{
		return 1;
	}

	switch (month)
	{
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
			days =  31;
			break;
		case 4: case 6: case 9: case 11:
			days =  30;
			break;
		case 2:
			if ( isLeapYear(year, 1) )
				days = 29;
			else
				days = 28;
			break;
	}
	
	days += juliusStartDay(year, month-1);
	return days;
}

int getMonthName(const char* month)
{
	int i;
	char temp[4];

	for (i = 0; i < 3; i++)
	{
		temp[i] = tolower(month[i]);
	}
	temp[3] = '\0';

	if ( strcmp(temp, "jan") == 0 )
		return 1;
	if ( strcmp(temp, "feb") == 0 )
		return 2;
	if ( strcmp(temp, "mar") == 0 )
		return 3;
	if ( strcmp(temp, "apr") == 0 )
		return 4;
	if ( strcmp(temp, "may") == 0 )
		return 5;
	if ( strcmp(temp, "jun") == 0 )
		return 6;
	if ( strcmp(temp, "jul") == 0 )
		return 7;
	if ( strcmp(temp, "aug") == 0 )
		return 8;
	if ( strcmp(temp, "sep") == 0 )
		return 9;
	if ( strcmp(temp, "oct") == 0 )
		return 10;
	if ( strcmp(temp, "nov") == 0 )
		return 11;
	if ( strcmp(temp, "dec") == 0 )
		return 12;

	return 0;
}

void printCalendar(int year, int month, int opt)
{
	int i, j, days;
	int newline = 0;
	int weekDay = getStartWeekDay(year, month, opt);
	if (opt)
	{
		printf("         %d월 %d\n", month, year);
		printf(" 일  월  화  수  목  금  토\n");
		i = juliusStartDay(year, month-1);
		days = getDays(year, month, opt);

		for(j = 0; j < getStartWeekDay(year, month, opt); j++)
		{
			printf("    ");
			newline++;
		}

		while ( i <  days + juliusStartDay(year, month-1))
		{
			if (i < 10)
				printf("  %d ", i);			
			else if ( i < 100 ) 
				printf(" %d ", i);
			else
			{
				if (year == 1752 && month == 9 && i > 246)
					printf("%d ", i + 11);
				else
					printf("%d ", i);
			}
			newline++;
			if ( newline % 7 == 0)
				putchar('\n');
			
			i++;
		}
	}
	else
	{
		printf("      %d월 %d\n", month, year);
		printf("일 월 화 수 목 금 토\n");
		i = 1;
		days = getDays(year, month, opt);

		for(j = 0; j < getStartWeekDay(year, month, opt); j++)
		{
			printf("   ");
			newline++;
		}

		while ( i <=  days)
		{
			
			if (i < 10)
			{
				if (year == 1752 && month == 9 && i > 2)
					printf("%d ", i + 11);
				else
					printf(" %d ", i);
			}
			else
			{
				if (year == 1752 && month == 9 && i > 2)
					printf("%d ", i + 11);
				else
					printf("%d ", i);
			}


			newline++;
			if ( newline % 7 == 0)
				putchar('\n');
			
			i++;
		}
	}

	if ( newline%7 !=0 )
		putchar('\n');

	putchar('\n');
}

void printYearCalendar(int year, int opt)
{
	int i;
	for(i = 1; i <= 12; i++)
	{
		printCalendar(year, i, opt);
	}
}

int main (int argc, char* argv[])
{
	int n, i, j;
	extern char* optarg;
	extern int optind;

	int juliusOpt = 0;				//defult Gregory ; julius boolean
	int yearOpt = 0;				//year boolean
	int yearSet = 0;				//whether there is year argument or not
	int monthSet = 0;				//whether there is month argument or not

	struct tm* tt;
	time_t t;

	int year;
	int month;

	int temp;
	
	time(&t);
	tt = gmtime(&t);
	year = tt->tm_year + 1900;
	month = tt->tm_mon + 1;

	while (( n = getopt(argc, argv, "jy")) != -1)
	{
		switch (n) 
		{
			case 'j':
				juliusOpt = 1;
				break;
			case 'y':
				yearOpt = 1;
				break;
		}
	}

	for( i = argc-1; i > 0; i--)
	{
		if (getMonthName(argv[i]))	//check month name
		{
			if ( yearSet )
			{
				temp = getMonthName(argv[i]);
			}
			else
			{
				printf("not a valid year %s\n", argv[i]);
				return -1;
			}
		}
		else
		{
			temp = atoi(argv[i]);
		}

		if (temp != 0)			//check not number type except from month name
		{
			if (yearSet)		//check year set or not
			{
				if (temp > 0 && temp < 13)
				{
					month = temp;
					monthSet = 1;
				}
				else
				{
					printf("cal: %d is neither a month number (1..12) nor a name\n", temp);
					return 0;
				}
				break;
			}
			else
			{
				if ( temp > 0 && temp < 10000 )
				{
					year = temp;
					yearSet = 1;
				}
				else
				{
					printf("cal: year '%d' not in range 1..9999\n", temp);
					return -1;
				}
			}
		}		
	}

	//printf ("julius:%d\nyearOpt:%d\nyaer:%d\nmonth:%d\n", juliusOpt, yearOpt, year, month);
	if ( yearOpt || (monthSet == 0 && yearSet == 1) )
	{
		printYearCalendar(year, juliusOpt);
	}
	else
	{
		printCalendar(year, month, juliusOpt);
	}

	return 0;
}
