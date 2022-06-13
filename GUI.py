import subprocess
# import psutil
import time
import tkinter as tk
import tkinter.ttk as ttk


def call(IP):
    ip =  ""
    if IP.get() != "":
        ip = IP.get()

    process = subprocess.Popen(f'./bin/discorb.out {ip}', shell=True)
    time.sleep(3)
    process.kill()
    return 

# def search():
#     subprocess.run('./communication 157.82.207.220 50000 | play -t raw -r 44100 -e s -c 1 -b 16 -', shell=True)

def cancel():
    # pids = {
    #     p.info["name"]:p.info["pid"]
    #     for p in psutil.process_iter(attrs=["pid","name"])
    # }
    # subprocess.run(f'kill {pids["rec"]}',shell=True)
    # subprocess.run(f'kill {pids["play"]}',shell=True)
    root.destroy()



# rootメインウィンドウの設定
root = tk.Tk()
root.title("phone")
root.geometry("200x200")

# メインフレームの作成と設置
frame = ttk.Frame(root)
frame.pack(fill = tk.BOTH, padx=20,pady=10)

# # StringVarのインスタンスを格納する変数textの設定
# text = tk.StringVar(frame)
# text.set("Call")

# 各種ウィジェットの作成
# 電話かける

IPaddresslabel = tk.Label(text="IPaddress")
IPaddresslabel.pack()
IPaddress = tk.Entry()
IPaddress.pack()

button = tk.Button(frame, text="Call", command=lambda:call(IPaddress))
button.pack(side="left")


#電話切る
button2 = tk.Button(frame, text="Cancel", command=lambda:cancel())
# button2 = tk.Button(frame, text="Cancel")
button2.pack(side='right')






root.mainloop()





