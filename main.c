#include <proto/muimaster.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/alib.h>
#include <proto/exec.h>
#include <proto/mathtrans.h>
#include <exec/rawfmt.h>
#include <proto/mathffp.h>

#include <clib/alib_protos.h>
#include <clib/mathffp_protos.h>
#include <clib/mathtrans_protos.h>

#define CALM_Button_Pressed 0xAEAEAE01
#define CALM_Show 			0xAEAEAE02
#define CALM_Hide 			0xAEAEAE03

#define CONNECT_BUTTON(button, name)\
	DoMethod(button, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2,\
		CALM_Button_Pressed, (ULONG)name);

#define CONNECT_KEY(key, name)\
	DoMethod(app_data->Win, MUIM_Notify, MUIA_Window_InputEvent, key, obj, 2,\
		CALM_Button_Pressed, (ULONG)name);

#define CONNECT_BUTTON_KEY(button, key, name)\
	CONNECT_BUTTON(button, name)\
	CONNECT_KEY(key,  name);
		
#pragma pack()
struct button_args 
{
	ULONG MethodID;
	ULONG Button;
};		

enum calc_button 
{
	CALC_BUTTON_NONE = -1,
	CALC_BUTTON_ZERO = 0,
	CALC_BUTTON_ONE,
	CALC_BUTTON_TWO,
	CALC_BUTTON_THREE,
	CALC_BUTTON_FOUR,
	CALC_BUTTON_FIVE,
	CALC_BUTTON_SIX,
	CALC_BUTTON_SEVEN,
	CALC_BUTTON_EIGHT,
	CALC_BUTTON_NINE,
	CALC_BUTTON_DOT,
	CALC_BUTTON_EQ,
	CALC_BUTTON_DIV,
	CALC_BUTTON_MUL,
	CALC_BUTTON_ADD,
	CALC_BUTTON_SUB,
	CALC_BUTTON_BSPACE,
	CALC_BUTTON_RESET
};

enum calc_error 
{
	CALC_ERROR_NOLIBTRANS = -999
};

Object *App, *Win;

struct MUI_CustomClass *CALC_Application = NULL;
IPTR ApplicationDispatcher(void);
const struct EmulLibEntry ApplicationGate = {TRAP_LIB, 0, (void(*)(void))ApplicationDispatcher};

#define PLACES 12
struct ApplicationData
{
	Object *Win, *Text, *One, *Two, *Three, *Four, *Five, *Six, *Seven, *Eight, *Nine, *Multiply, *Divide, *Dot, *Subtract, *Add, *Equals, *Zero;
	char display_value[PLACES];
	int active_place;
	double working_value;
	BOOL initial_operation;
	enum calc_button active_operation, new_operation;
};

void ApplicationResetDisplayValue(struct ApplicationData *app_data);

struct MUI_CustomClass *CreateApplicationClass(void)
{
	struct MUI_CustomClass *cl;
	cl = MUI_CreateCustomClass(NULL, MUIC_Application, NULL, sizeof(struct ApplicationData), (APTR)&ApplicationGate);
	CALC_Application = cl;
	return cl;	
}

void DeleteApplicationClass(void)
{
	if (CALC_Application)
	{
		MUI_DeleteCustomClass(CALC_Application);
	}
}

Object *ApplicationWindow(struct ApplicationData *app_data)
{
	return (app_data->Win = MUI_NewObject(MUIC_Window,
			MUIA_Window_Title, (ULONG)"MUICalc",
			MUIA_Window_ID, (ULONG)"winny",
			MUIA_Window_Activate, TRUE,
			MUIA_Window_Height, 100,
			MUIA_Window_Width, 200,
			MUIA_Window_CloseGadget, TRUE,
			MUIA_Window_RootObject, MUI_NewObject(MUIC_Group,
				MUIA_Group_Horiz, FALSE,
				MUIA_Group_Child, (app_data->Text = MUI_NewObject(MUIC_Text,
					MUIA_Text_Contents, "0.0",
				TAG_END)),
				MUIA_Group_Child, MUI_NewObject(MUIC_Group,
					MUIA_Group_Horiz, TRUE,
					MUIA_Group_Child, (app_data->One = MUI_MakeObject(MUIO_Button,
						"1",				
					TAG_END)),
					MUIA_Group_Child, (app_data->Two = MUI_MakeObject(MUIO_Button,
						"2",
					TAG_END)),
					MUIA_Group_Child, (app_data->Three = MUI_MakeObject(MUIO_Button,
						"3",
					TAG_END)),
					MUIA_Group_Child, (app_data->Multiply = MUI_MakeObject(MUIO_Button,
						"*",
					TAG_END)),
				TAG_END),
				MUIA_Group_Child, MUI_NewObject(MUIC_Group,
					MUIA_Group_Horiz, TRUE,
					MUIA_Group_Child, (app_data->Four = MUI_MakeObject(MUIO_Button,
						"4",				
					TAG_END)),
					MUIA_Group_Child, (app_data->Five = MUI_MakeObject(MUIO_Button,
						"5",
					TAG_END)),
					MUIA_Group_Child, (app_data->Six = MUI_MakeObject(MUIO_Button,
						"6",
					TAG_END)),
					MUIA_Group_Child, (app_data->Divide = MUI_MakeObject(MUIO_Button,
						"/",
					TAG_END)),
				TAG_END),
				MUIA_Group_Child, MUI_NewObject(MUIC_Group,
					MUIA_Group_Horiz, TRUE,
					MUIA_Group_Child, (app_data->Seven = MUI_MakeObject(MUIO_Button,
						"7",				
					TAG_END)),
					MUIA_Group_Child, (app_data->Eight = MUI_MakeObject(MUIO_Button,
						"8",
					TAG_END)),
					MUIA_Group_Child, (app_data->Nine = MUI_MakeObject(MUIO_Button,
						"9",
					TAG_END)),
					MUIA_Group_Child, (app_data->Subtract = MUI_MakeObject(MUIO_Button,
						"-",
					TAG_END)),
				TAG_END),
				MUIA_Group_Child, MUI_NewObject(MUIC_Group,
					MUIA_Group_Horiz, TRUE,
					MUIA_Group_Child, (app_data->Dot = MUI_MakeObject(MUIO_Button,
						".",				
					TAG_END)),
					MUIA_Group_Child, (app_data->Zero = MUI_MakeObject(MUIO_Button,
						"0",
					TAG_END)),
					MUIA_Group_Child, (app_data->Equals = MUI_MakeObject(MUIO_Button,
						"=",
					TAG_END)),
					MUIA_Group_Child, (app_data->Add = MUI_MakeObject(MUIO_Button,
						"+",
					TAG_END)),
				TAG_END),
			TAG_END),
		TAG_END));
}

IPTR ApplicationNew(Class *cl, Object *obj, struct opSet *msg)
{
	Printf("App new called!\n");
	struct ApplicationData preapp_data;
	struct TagItem params[] = {
		{ MUIA_Application_Author, (ULONG)"Meguca" },
		{ MUIA_Application_Base, (ULONG)"CALC" },
		{ MUIA_Application_Title, (ULONG)"Calc" },
		{ MUIA_Application_Window, (ULONG)ApplicationWindow(&preapp_data) },
		{ TAG_END, 0 }
	};
	msg->ops_AttrList = params;
	obj = (Object*)DoSuperMethodA(cl, obj, (Msg)msg);
	struct ApplicationData *app_data = INST_DATA(cl, obj);
	CopyMem(&preapp_data, app_data, sizeof preapp_data);
	app_data->working_value = 0.0;
	ApplicationResetDisplayValue(app_data);
	app_data->active_place = 0;
	app_data->initial_operation = TRUE;
	app_data->active_operation = CALC_BUTTON_NONE;
	app_data->new_operation = CALC_BUTTON_NONE;
	DoMethod(app_data->Win, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 2,
		MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
	
	CONNECT_BUTTON_KEY(app_data->One, "1", CALC_BUTTON_ONE);
	CONNECT_BUTTON_KEY(app_data->Two, "2", CALC_BUTTON_TWO);
	CONNECT_BUTTON_KEY(app_data->Three, "3", CALC_BUTTON_THREE);
	CONNECT_BUTTON_KEY(app_data->Four, "4", CALC_BUTTON_FOUR);
	CONNECT_BUTTON_KEY(app_data->Five, "5", CALC_BUTTON_FIVE);
	CONNECT_BUTTON_KEY(app_data->Six, "6", CALC_BUTTON_SIX);
	CONNECT_BUTTON_KEY(app_data->Seven, "7", CALC_BUTTON_SEVEN);
	CONNECT_BUTTON_KEY(app_data->Eight, "8", CALC_BUTTON_EIGHT);
	CONNECT_BUTTON_KEY(app_data->Nine, "9", CALC_BUTTON_NINE);
	CONNECT_BUTTON_KEY(app_data->Add, "+", CALC_BUTTON_ADD);
	CONNECT_BUTTON_KEY(app_data->Subtract, "-", CALC_BUTTON_SUB);
	// * doesn't work!
	CONNECT_BUTTON_KEY(app_data->Multiply, "x", CALC_BUTTON_MUL);
	CONNECT_BUTTON_KEY(app_data->Divide, "/", CALC_BUTTON_DIV);
	CONNECT_BUTTON_KEY(app_data->Dot, ".", CALC_BUTTON_DOT);
	CONNECT_BUTTON_KEY(app_data->Equals, "return", CALC_BUTTON_EQ);
	CONNECT_BUTTON_KEY(app_data->Zero, "0", CALC_BUTTON_ZERO);
	CONNECT_KEY("backspace", CALC_BUTTON_BSPACE);
	CONNECT_KEY("r", CALC_BUTTON_RESET);
	
	return (IPTR)obj;
}


double string_to_double(char *string)
{
	if ((MathBase = OpenLibrary((STRPTR)"mathffp.library", 33)))
	{
		CloseLibrary(MathBase);
		return afp(string);
	}
	else
	{
		Printf("Couldn't open mathffp\n");
		return -1;
	}
}


BOOL ApplicationApplyOperation(struct ApplicationData *app_data)
{
	char buf_working[PLACES];
	char buf_display[PLACES];
	double result = 0;
	double working = app_data->working_value;
	double display = string_to_double(app_data->display_value);
	NewRawDoFmt("%f", RAWFMTFUNC_STRING, buf_working, working);
	NewRawDoFmt("%f", RAWFMTFUNC_STRING, buf_display, display);
	Printf("Working value is %s, display value is %s\n", buf_working, buf_display);
	switch (app_data->active_operation)
	{
		case CALC_BUTTON_MUL:
			Printf("Op is MUL\n");
			result = working * display;
			break;
		case CALC_BUTTON_DIV:
			Printf("Op is DIV\n");
			if (display == 0) 
				return FALSE;
			else
				result = working / display;
			break;
		case CALC_BUTTON_ADD:
			Printf("Op is ADD\n");
			result = working + display;
			break;
		case CALC_BUTTON_SUB:
			Printf("Op is sub\n");
			result = working - display;
			break;
		default:
			break;
	}
	NewRawDoFmt("%f", RAWFMTFUNC_STRING, buf_working, result);
	Printf("Result is %s\n", buf_working);
	app_data->working_value = result;
	return TRUE;
}


void ApplicationAddToDisplayValue(struct ApplicationData *app_data, char value)
{
	Printf("Adding %ld to display\n", value);
	if (app_data->active_place > PLACES - 1) 
	{
		app_data->active_place = PLACES - 1;
	}
	app_data->display_value[(app_data->active_place)++] = value + 48;
	app_data->display_value[PLACES - 1] = '\0';
}

void ApplicationAddDotToDisplayValue(struct ApplicationData *app_data)
{
	Printf("Adding dot to display\n");
	if (app_data->active_place > PLACES - 1)
	{
		app_data->active_place = PLACES - 1;
	}
	app_data->display_value[(app_data->active_place)++] = '.';
	app_data->display_value[PLACES - 1] = '\0';
}


void ApplicationSetDisplayValueFromWorking(struct ApplicationData *app_data)
{
	char text[80];
	int i;
	
	NewRawDoFmt("%f", RAWFMTFUNC_STRING, text, app_data->working_value);
	for (i = 0; i < PLACES; i++)
	{
		app_data->display_value[i] = text[i];
	}
}

void ApplicationSetWorkingValueFromDisplay(struct ApplicationData *app_data)
{
	char buff[PLACES];
	double value = string_to_double(app_data->display_value);
	app_data->working_value = value;
	NewRawDoFmt("%f", RAWFMTFUNC_STRING, buff, value);
	Printf("Set working value to %s, from display %s\n", buff, app_data->display_value);	
}

void ApplicationResetDisplayValue(struct ApplicationData *app_data)
{
	int i;
	for (i = 0; i < PLACES; i++)
	{
		app_data->display_value[i] = '\0';
	}
	app_data->active_place = 0;
}

BOOL ApplicationDisplayValueContainsDot(struct ApplicationData *app_data) 
{
	int i;
	for (i = 0; i < PLACES; i++)
	{
		if (app_data->display_value[i] == '.')
		{
			return TRUE;
		}
	}
	return FALSE;
}

void ApplicationResetToError(struct ApplicationData *app_data)
{
	app_data->display_value[0] = 'E';
	app_data->display_value[1] = 'R';
	app_data->display_value[2] = 'R';
	app_data->display_value[3] = '\0';
	set(app_data->Text, MUIA_Text_Contents, app_data->display_value);	
}

void ApplicationDeleteDisplayCharacter(struct ApplicationData *app_data)
{
	app_data->active_place--;
	if (app_data->active_place < 0)
	{
		app_data->active_place = 0;
	}
	app_data->display_value[app_data->active_place] = '\0';
}

void ApplicationReset(struct ApplicationData *app_data)
{
	ApplicationResetDisplayValue(app_data);
	app_data->new_operation = CALC_BUTTON_NONE;
	app_data->working_value = 0;
	app_data->active_operation = CALC_BUTTON_NONE;
	app_data->new_operation = CALC_BUTTON_NONE;
}

IPTR ApplicationButtonPressed(Class *cl, Object *obj, struct button_args *msg)
{
	Printf("Button Pressed %ld!\n", msg->Button);
	struct ApplicationData *app_data = INST_DATA(cl, obj);
	
	if (msg->Button < 10)
	{
		if (app_data->new_operation == CALC_BUTTON_EQ)
		{
			ApplicationResetDisplayValue(app_data);
			ApplicationAddToDisplayValue(app_data, msg->Button);
			app_data->new_operation = CALC_BUTTON_NONE;
		}
		else if (app_data->new_operation == CALC_BUTTON_DOT && !ApplicationDisplayValueContainsDot(app_data))
		{
			ApplicationAddDotToDisplayValue(app_data);
			ApplicationAddToDisplayValue(app_data, msg->Button);
			app_data->new_operation = CALC_BUTTON_NONE;
		} 
		else if (app_data->new_operation != CALC_BUTTON_NONE)
		{
			if (!app_data->initial_operation) 
			{
				if (!ApplicationApplyOperation(app_data))
				{
					ApplicationResetToError(app_data);
					return 0;
				}
			}
			else
			{
				app_data->initial_operation = FALSE;
				ApplicationSetWorkingValueFromDisplay(app_data);
			}
			ApplicationResetDisplayValue(app_data);
			ApplicationAddToDisplayValue(app_data, msg->Button);
			app_data->active_operation = app_data->new_operation;
			app_data->new_operation = CALC_BUTTON_NONE;
		}
		else 
		{
			ApplicationAddToDisplayValue(app_data, msg->Button);
		}
	}
	else
	{
		switch(msg->Button)
		{
		case CALC_BUTTON_MUL:
		case CALC_BUTTON_DIV:
		case CALC_BUTTON_DOT:
		case CALC_BUTTON_ADD:
		case CALC_BUTTON_SUB:
			app_data->new_operation = msg->Button;
			break;
		case CALC_BUTTON_EQ:
			if (!ApplicationApplyOperation(app_data))
			{
				ApplicationResetToError(app_data);
				return 0;
			}
			Printf("Working is %ld\n", (long)app_data->working_value);
			ApplicationSetDisplayValueFromWorking(app_data);
			app_data->working_value = 0;
			app_data->initial_operation = TRUE;
			app_data->active_operation = CALC_BUTTON_NONE;
			app_data->new_operation = CALC_BUTTON_EQ;
			break;
		case CALC_BUTTON_BSPACE:
			ApplicationDeleteDisplayCharacter(app_data);	
			break;
		case CALC_BUTTON_RESET:
			ApplicationReset(app_data);
			break;
		default:
			break;
		}
	}
	// Display changes in UI
	set(app_data->Text, MUIA_Text_Contents, app_data->display_value);
	Printf("working %ld, display '%s', active op %ld, new op %ld\n", (long)app_data->working_value, app_data->display_value, app_data->active_operation, app_data->new_operation);
	return 0;
}

IPTR ApplicationDispatcher(void)
{
	Class *cl = (Class*)REG_A0;
	Object *obj = (Object*)REG_A2;
	Msg msg = (Msg)REG_A1;
	struct ApplicationData *app_data = INST_DATA(cl, obj);
	
	switch (msg->MethodID)
	{
	case OM_NEW:	
		return ApplicationNew(cl, obj, (struct opSet*)msg);
	case CALM_Show: 
		Printf("Show!\n");
		return set(app_data->Win, MUIA_Window_Open, TRUE);
	case CALM_Hide:
		Printf("Hide!\n");
		return set(app_data->Win, MUIA_Window_Open, FALSE);
	case CALM_Button_Pressed:
		return ApplicationButtonPressed(cl, obj, (struct button_args*)msg);
	default:		
		return DoSuperMethodA(cl, obj, msg);
	}
}


void setup_notifications(void)
{


}


Object* create_app(void)
{
	return NewObject(CALC_Application->mcc_Class, NULL, TAG_END);
}

void main_loop(void)
{
	ULONG signals = 0;
	DoMethod(App, CALM_Show);
	while (DoMethod(App, MUIM_Application_NewInput, &signals) != (ULONG)MUIV_Application_ReturnID_Quit)
	{
		signals = Wait(signals | SIGBREAKF_CTRL_C);
		if (signals & SIGBREAKF_CTRL_C) break;
	}
	DoMethod(App, CALM_Hide);
}

int main(void)
{
	if(CreateApplicationClass())
	{
		Printf("Custom class created %ld.\n", (LONG)CALC_Application);
		if((App = create_app()))
		{
			Printf("App created\n");
			setup_notifications();
			Printf("Notifications registered\n");
			main_loop();
			DisposeObject(App);
		}
	}
	
	return 0;
}
