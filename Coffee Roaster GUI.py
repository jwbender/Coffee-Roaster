from guizero import App, Text, TextBox, PushButton, Slider, Window, Box


def start_program():
    startwdw.hide()
    window1.show()

def start_manual():
    window1.hide()
    
def start_auto():
    window1.hide()
    
app = App(title="Coffee Roaster", width=480, height=320, layout="grid", bg="white")
#Start Screen
startwdw=Window(app, title="Coffee Roaster",width=480, height=320, layout="grid", bg="white")
startwdw.tk.attributes("-fullscreen", True)
startwdw.show(wait=True) #welcome window
space=Text(startwdw, text="",grid=[0,0], size=5) 
welcome_message = Text(startwdw, text=" Coffee Roaster 2.0 ", grid=[0,2], align ="left", size=40, font="Helvetica", color="sienna4")
space=Text(startwdw, text="",grid=[0,3], size=10, align="bottom")
startbt=PushButton(startwdw, text="START", command=start_program, grid=[0,4],image="giantbean.gif")
startbt.bg = "black"
startbt.height =170
startbt.width  =225 
space=Text(startwdw, text="Click Bean to START",grid=[0,5], size=26)

window1=Window(app, title="Coffee Roaster",width=480, height=320, layout="grid", bg="white")
window1.hide()
window1.tk.attributes("-fullscreen", True)
#window1.show(wait=True) #welcome window
space=Text(window1, text="",grid=[0,0], size=3) 
Text1 = Text(window1, text="     Select Roast", grid=[0,2], align ="left", size=40, font="Helvetica") 
Text1 = Text(window1, text="       Operation", grid=[0,3], align ="left", size=40, font="Helvetica") 
space=Text(window1, text="",grid=[0,4], size=15)
box=Box(window1, layout="grid",grid=[0,5])
space=Text(box, text="     ",grid=[0,0], size=20)
windowclose=PushButton(box, text="MANUAL", command=start_manual, grid=[1,0], align = "left")
windowclose.bg = "yellow2"
windowclose.height =3
windowclose.width  =8
windowclose.text_size=20
space=Text(box, text="       ",grid=[2,0], size=20)
windowclose=PushButton(box, text="AUTO", command=start_auto, grid=[3,0])
windowclose.bg = "lawn green"
windowclose.height =3
windowclose.width  =8
windowclose.text_size=20


app.display()