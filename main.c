/* compile with "gcc -noixemul -o helloworld_mui helloworld_mui.c" */

#include <proto/muimaster.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/alib.h>
#include <proto/exec.h>

#define CALM_Button_Pressed 0xAEAEAE01
#define CALM_Show 			0xAEAEAE02
#define CALM_Hide 			0xAEAEAE03

#define CONNECT_BUTTON(button, name) DoMethod(button, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2,\
		CALM_Button_Pressed, (ULONG)name);
		
#pragma pack()
struct button_args {
	ULONG MethodID;
	ULONG Button;
};		

enum calc_button {
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

Object *App, *Win;


struct MUI_CustomClass *CALC_Application = NULL;
IPTR ApplicationDispatcher(void);
const struct EmulLibEntry ApplicationGate = {TRAP_LIB, 0, (void(*)(void))ApplicationDispatcher};

struct ApplicationData
{
	Object *Win, *One, *Two, *Three, *Four, *Multiply, *Divide, *Dot, *Equals, *Zero;
};

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
				MUIA_Group_Child, MUI_NewObject(MUIC_Text,
					MUIA_Text_Contents, "0.0",
				TAG_END),
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


IPTR ApplicationButtonPressed(Class *cl, Object *obj, struct button_args *msg)
{
	Printf("Button Pressed!\n");
	if (msg->Button < 5)
	{
		Printf("Pressed number: %ld\n", msg->Button);
	}
	else
	{
		switch(msg->Button)
		{
			case CALC_BUTTON_MUL:
				Printf("Mul\n");
				break;
			case CALC_BUTTON_DIV:
				Printf("Div\n");
				break;
			case CALC_BUTTON_EQ:
				Printf("Eq\n");
			case CALC_BUTTON_DOT:
				Printf("Dot\n");
			default:
				Printf("Not implemented\n");
		}
	}

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
