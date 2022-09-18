#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <libxfce4panel/libxfce4panel.h>

#define DRIVE_NAME "sda"
#define DRIVE_SECTOR_SIZE_B 512
#define INTERVAL_S 3
#define RW_SPEED_BS_DIGIT_COUNT 9
#define UL_MAX_DIGIT_COUNT 20

struct DriveRWTotal
{
	unsigned long rb;
	unsigned long wb;
};

struct DriveRWSpeed
{
	unsigned rsbs;
	unsigned wsbs;
};

struct DriveRWTotalRaw
{
	char rss[UL_MAX_DIGIT_COUNT];
	char wss[UL_MAX_DIGIT_COUNT];
};

struct DriveRWSpeedString
{
	char rsbss[RW_SPEED_BS_DIGIT_COUNT + 1];
	char wsbss[RW_SPEED_BS_DIGIT_COUNT + 1];
};

struct DriveRWTotal lastDriveRWTotal;
struct DriveRWSpeed driveRWSpeed;
struct DriveRWTotalRaw driveRWTotalRaw;
struct DriveRWSpeedString driveRWSpeedString;
bool isLastSet;
char driveNameLen;
char* mon;
GtkWidget* lblMon;
XfcePanelPlugin* plugin;

void u2sfdc(unsigned u, char c, char* s);
void updateDriveRWSpeed(void);
void showDriveRWSpeed(void);
void timerTick(void);
gboolean timerTickGtkWrapper(gpointer);
void startTimer(void);
void initVars(void);
void initGUI(void);
void initProg(void);
void driverwmonConstruct(XfcePanelPlugin* p);

XFCE_PANEL_PLUGIN_REGISTER(driverwmonConstruct)

void u2sfdc(unsigned u, char c, char* s)
{
	char d;

	while(c-- > 0)
	{
		if(u == 0U)
			s[c] = '0';
		else
		{
			d = u % 10U;
			u /= 10U;

			s[c] = '0' + d;
		}
	}
}

void updateDriveRWSpeed(void)
{
	struct DriveRWTotal drwt;
	bool dnm, ics;
	char c, ssi;
	unsigned i, j, id, sc;
	char* ssp;
	FILE* dsf;

	dnm = false;
	i = 0U;
	dsf = fopen("/proc/diskstats", "r");

	for(;(c = getc(dsf)) != EOF; i++)
	{
		if(!dnm)
			j = i;

		id = i - j;

		if(id < driveNameLen)
			dnm = c == DRIVE_NAME[id];
		else
		{
			if(id == driveNameLen)
				sc = 0U;

			ics = c == ' ';

			switch(sc)
			{
			case 3:
				ssp = &(driveRWTotalRaw.rss[0]);
				break;
			case 7:
				ssp = &(driveRWTotalRaw.wss[0]);
				break;
			default:
				ssp = NULL;
			}

			if(ssp != NULL)
			{
				if(ics)
					ssp[ssi] = '\0';
				else
					ssp[ssi++] = c;
			}

			if(ics)
			{
				if(sc >= 7)
					break;

				ssi = 0;
				sc++;
			}
		}
	}

	fclose(dsf);

	drwt.rb = atoi(driveRWTotalRaw.rss) * DRIVE_SECTOR_SIZE_B;
	drwt.wb = atoi(driveRWTotalRaw.wss) * DRIVE_SECTOR_SIZE_B;

	if(isLastSet)
	{
		driveRWSpeed.rsbs = (drwt.rb - lastDriveRWTotal.rb) / INTERVAL_S;
		driveRWSpeed.wsbs = (drwt.wb - lastDriveRWTotal.wb) / INTERVAL_S;
	}
	else
	{
		driveRWSpeed.rsbs = driveRWSpeed.wsbs = 0U;
		isLastSet = true;
	}

	lastDriveRWTotal = drwt;
}

void showDriveRWSpeed(void)
{
	u2sfdc(driveRWSpeed.rsbs, RW_SPEED_BS_DIGIT_COUNT, driveRWSpeedString.rsbss);
	u2sfdc(driveRWSpeed.wsbs, RW_SPEED_BS_DIGIT_COUNT, driveRWSpeedString.wsbss);

	sprintf(mon, "[%s] r: %s B/s w: %s B/s", DRIVE_NAME, driveRWSpeedString.rsbss, driveRWSpeedString.wsbss);

	gtk_label_set_text(GTK_LABEL(lblMon), mon);
}

void timerTick(void)
{
	updateDriveRWSpeed();
	showDriveRWSpeed();
}

gboolean timerTickGtkWrapper(gpointer)
{
	timerTick();
	return TRUE;
}

void startTimer(void)
{
	timerTick();
	g_timeout_add_seconds(INTERVAL_S, timerTickGtkWrapper, NULL);
}

void initVars(void)
{
	isLastSet = false;
	driveNameLen = strlen(DRIVE_NAME);

	driveRWSpeedString.rsbss[RW_SPEED_BS_DIGIT_COUNT] =
		driveRWSpeedString.wsbss[RW_SPEED_BS_DIGIT_COUNT] =
		'\0';

	mon = malloc(19 + driveNameLen + 2 * RW_SPEED_BS_DIGIT_COUNT);
}

void initGUI(void)
{
	lblMon = gtk_label_new("");
	gtk_container_add(GTK_CONTAINER(plugin), lblMon);
	gtk_widget_show(lblMon);
}

void initProg(void)
{
	initVars();
	initGUI();
}

void driverwmonConstruct(XfcePanelPlugin* p)
{
	plugin = p;

	initProg();
	startTimer();
}

