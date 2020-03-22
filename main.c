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

#define CONNECT_BUTTON(button, name) DoMethod(button, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2,\
		CALM_Button_Pressed, (ULONG)name);
		
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
	CALC_BUTTON_EQ,
	CALC_BUTTON_DIV,
	CALC_BUTTON_MUL,
	CALC_BUTTON_DOT
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
	Object *Win, *Text, *One, *Two, *Three, *Four, *Multiply, *Divide, *Dot, *Equals, *Zero;
	char display_value[PLACES];
	int active_place;
	long working_value;
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
			MUIA_Window_Title, (ULONG)"My window..",
			MUIA_Window_ID, (ULONG)"winny",
			MUIA_Window_Activate, TRUE,
			MUIA_Window_Height, 100,
			MUIA_Window_Width, 100,
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
					MUIA_Group_Child, (app_data->Multiply = MUI_MakeObject(MUIO_Button,
						"*",
					TAG_END)),
				TAG_END),
				MUIA_Group_Child, MUI_NewObject(MUIC_Group,
					MUIA_Group_Horiz, TRUE,
					MUIA_Group_Child, (app_data->Three = MUI_MakeObject(MUIO_Button,
						"3",				
					TAG_END)),
					MUIA_Group_Child, (app_data->Four = MUI_MakeObject(MUIO_Button,
						"4",
					TAG_END)),
					MUIA_Group_Child, (app_data->Divide = MUI_MakeObject(MUIO_Button,
						"/",
					TAG_END)),
				TAG_END),
				MUIA_Group_Child, MUI_NewObject(MUIC_Group,
					MUIA_Group_Horiz, TRUE,
					MUIA_Group_Child, (app_data->Zero = MUI_MakeObject(MUIO_Button,
						"0",				
					TAG_END)),
					MUIA_Group_Child, (app_data->Dot = MUI_MakeObject(MUIO_Button,
						".",
					TAG_END)),
					MUIA_Group_Child, (app_data->Equals = MUI_MakeObject(MUIO_Button,
						"=",
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
	app_data->working_value = 0;
	ApplicationResetDisplayValue(app_data);
	app_data->active_place = 0;
	app_data->initial_operation = TRUE;
	app_data->active_operation = CALC_BUTTON_NONE;
	app_data->new_operation = CALC_BUTTON_NONE;
	DoMethod(app_data->Win, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 2,
		MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
	CONNECT_BUTTON(app_data->One, CALC_BUTTON_ONE);
	CONNECT_BUTTON(app_data->Two, CALC_BUTTON_TWO);
	CONNECT_BUTTON(app_data->Three, CALC_BUTTON_THREE);
	CONNECT_BUTTON(app_data->Four, CALC_BUTTON_FOUR);
	CONNECT_BUTTON(app_data->Multiply, CALC_BUTTON_MUL);
	CONNECT_BUTTON(app_data->Divide, CALC_BUTTON_DIV);
	CONNECT_BUTTON(app_data->Dot, CALC_BUTTON_DOT);
	CONNECT_BUTTON(app_data->Equals, CALC_BUTTON_EQ);
	CONNECT_BUTTON(app_data->Zero, CALC_BUTTON_ZERO);
	
	return (IPTR)obj;
}



int slow_pow(int base, int pow) 
{
	int i = 0;
	int result = 1;
	for (; i < pow; i++)
	{
		result *= base;
	}
	return result;
}

int string_to_int(char *string)
{
	int result = 0;
	int i = 0;
	int len = 0;
	
	while(string[len] != '\0')
	{
		len++;
	}
	Printf("len of '%s' %ld\n", string, len);
	for(i = 0; i < len; i++)
	{
		Printf("Adding %ld * %ld\n", slow_pow(10, i), (int)(string[len - i - 1] - 48)); 
		result += slow_pow(10, i) * (int)(string[len - i - 1] - 48);
	}

	return result;
}


void ApplicationApplyOperation(struct ApplicationData *app_data)
{
	int result = 0;
	int working = app_data->working_value;
	int display = string_to_int(app_data->display_value);
	Printf("Working value is %ld, display value is %ld\n", working, display);
	switch (app_data->active_operation)
	{
		case CALC_BUTTON_MUL:
			Printf("Op is MUL\n");
			result = working * display;
			break;
		case CALC_BUTTON_DIV:
			Printf("Op is DIV\n");
			result = working / display;
			break;
		default:
			break;
	}
	Printf("Result is %ld\n", result);
	app_data->working_value = result;
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



void ApplicationSetDisplayValueFromWorking(struct ApplicationData *app_data)
{
	char text[PLACES];
	int i;
	
	NewRawDoFmt("%ld", RAWFMTFUNC_STRING, text, app_data->working_value);
	for (i = 0; i < PLACES; i++)
	{
		app_data->display_value[i] = text[i];
	}
}

void ApplicationSetWorkingValueFromDisplay(struct ApplicationData *app_data)
{
	int value = string_to_int(app_data->display_value);
	app_data->working_value = value;
	Printf("Set working value to %ld, from display %s\n", value, app_data->display_value);	
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

IPTR ApplicationButtonPressed(Class *cl, Object *obj, struct button_args *msg)
{
	Printf("Button Pressed %ld!\n", msg->Button);
	struct ApplicationData *app_data = INST_DATA(cl, obj);
	
	if (msg->Button < 5)
	{
		if (app_data->new_operation == CALC_BUTTON_EQ)
		{
			ApplicationResetDisplayValue(app_data);
			ApplicationAddToDisplayValue(app_data, msg->Button);
		}
		else if (app_data->new_operation == CALC_BUTTON_DOT && !ApplicationDisplayValueContainsDot(app_data))
		{
			ApplicationAddToDisplayValue(app_data, '.' - 48);
		} 
		else if (app_data->new_operation != CALC_BUTTON_NONE)
		{
			if (!app_data->initial_operation) 
			{
				ApplicationApplyOperation(app_data);
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
			app_data->new_operation = msg->Button;
			break;
		case CALC_BUTTON_EQ:
			ApplicationApplyOperation(app_data);
			ApplicationSetDisplayValueFromWorking(app_data);
			app_data->working_value = 0;
			app_data->initial_operation = TRUE;
			app_data->active_operation = CALC_BUTTON_NONE;
			app_data->new_operation = CALC_BUTTON_EQ;
			break;
		default:
			break;
		}
	}
	// Display changes in UI
	set(app_data->Text, MUIA_Text_Contents, app_data->display_value);
	Printf("working %ld, display '%s', active op %ld, new op %ld\n", app_data->working_value, app_data->display_value, app_data->active_operation, app_data->new_operation);
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
