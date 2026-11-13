
import os
import subprocess
import sys
import tkinter as tk

HERE = os.path.dirname(os.path.abspath(__file__))
EXE = os.path.join(HERE, "calc.exe" if os.name == "nt" else "calc")

# ---- colours ---------------------------------------------------------------
BG = "#14142b"
SCREEN = "#0b1e2d"


NUM = ("#2b2d5b", "#3d4080")
OP = ("#ff8a3d", "#ffa66b")
FUNC = ("#6c3fd1", "#8a5cf0")
CLEAR = ("#e5384f", "#ff5a70")
EQUAL = ("#12b886", "#38d9a9")
MODE = ("#1c7ed6", "#4dabf7")
TEXT = "#ffffff"

# (label, text to insert / action, colour)
ROWS = [
    [("sin", "sin(", FUNC), ("cos", "cos(", FUNC), ("tan", "tan(", FUNC), ("asin", "asin(", FUNC), ("acos", "acos(", FUNC)],
    [("sinh", "sinh(", FUNC), ("cosh", "cosh(", FUNC), ("tanh", "tanh(", FUNC), ("log", "log(", FUNC), ("ln", "ln(", FUNC)],
    [("√", "sqrt(", FUNC), ("xʸ", "^", FUNC), ("n!", "!", FUNC), ("eˣ", "exp(", FUNC), ("|x|", "abs(", FUNC)],
    [("(", "(", MODE), (")", ")", MODE), ("π", "pi", MODE), ("e", "e", MODE), ("%", "%", MODE)],
    [("7", "7", NUM), ("8", "8", NUM), ("9", "9", NUM), ("÷", "/", OP), ("AC", "AC", CLEAR)],
    [("4", "4", NUM), ("5", "5", NUM), ("6", "6", NUM), ("×", "*", OP), ("⌫", "DEL", CLEAR)],
    [("1", "1", NUM), ("2", "2", NUM), ("3", "3", NUM), ("−", "-", OP), ("Ans", "ANS", MODE)],
    [("0", "0", NUM), (".", ".", NUM), ("DEG", "MODE", MODE), ("+", "+", OP), ("=", "=", EQUAL)],
]


class Calculator(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Scientific Calculator  (C++ engine)")
        self.configure(bg=BG)
        self.resizable(False, False)

        self.expr = ""
        self.last = ""
        self.degrees = True
        self.just_solved = False
        self.mode_label = None

        # ---- display ----
        screen = tk.Frame(self, bg=SCREEN, highlightbackground="#3bc9db", highlightthickness=2)
        screen.pack(fill="x", padx=14, pady=(14, 8))
        self.mode_txt = tk.Label(screen, text="DEG", bg=SCREEN, fg="#3bc9db",
                                 font=("Segoe UI", 10, "bold"), anchor="w")
        self.mode_txt.pack(fill="x", padx=10, pady=(6, 0))
        self.line1 = tk.Label(screen, text="", bg=SCREEN, fg="#8fb8c9", anchor="e",
                              font=("Consolas", 16), wraplength=420)
        self.line1.pack(fill="x", padx=10)
        self.line2 = tk.Label(screen, text="0", bg=SCREEN, fg="#63e6be", anchor="e",
                              font=("Consolas", 34, "bold"))
        self.line2.pack(fill="x", padx=10, pady=(0, 8))

        # ---- buttons ----
        pad = tk.Frame(self, bg=BG)
        pad.pack(padx=10, pady=(0, 14))
        for r, row in enumerate(ROWS):
            for c, (label, value, (col, hover)) in enumerate(row):
                b = tk.Label(pad, text=label, bg=col, fg=TEXT, width=5, height=2,
                             font=("Segoe UI", 14, "bold"), cursor="hand2")
                b.grid(row=r, column=c, padx=4, pady=4, ipadx=4, ipady=2)
                b.bind("<Button-1>", lambda e, v=value: self.press(v))
                b.bind("<Enter>", lambda e, w=b, h=hover: w.config(bg=h))
                b.bind("<Leave>", lambda e, w=b, cc=col: w.config(bg=cc))
                if value == "MODE":
                    self.mode_label = b

        # ---- keyboard ----
        self.bind("<Key>", self.on_key)
        self.bind("<Return>", lambda e: self.press("="))
        self.bind("<BackSpace>", lambda e: self.press("DEL"))
        self.bind("<Escape>", lambda e: self.press("AC"))

    # ---- helpers ----
    @staticmethod
    def pretty(text):
        return text.replace("*", "×").replace("/", "÷").replace("pi", "π").replace("sqrt", "√")

    def refresh(self):
        self.line1.config(text=self.pretty(self.expr))

    def on_key(self, event):
        ch = event.char
        if ch and (ch in "0123456789.+-*/()^!%" or ch.isalpha()):
            self.press(ch)

    # ---- actions ----
    def press(self, v):
        if v == "AC":
            self.expr = ""
            self.line2.config(text="0", fg="#63e6be")
        elif v == "DEL":
            self.expr = self.expr[:-1]
        elif v == "MODE":
            self.degrees = not self.degrees
            txt = "DEG" if self.degrees else "RAD"
            self.mode_txt.config(text=txt)
            self.mode_label.config(text=txt)
        elif v == "ANS":
            if self.last:
                self.expr += self.last
        elif v == "=":
            self.solve()
            return
        else:
            # start fresh after a result unless continuing with an operator
            if self.just_solved and v not in "+-*/^!%":
                self.expr = ""
            self.expr += v
        self.just_solved = False
        self.refresh()

    def solve(self):
        if not self.expr:
            return
        expr = self.expr + ")" * (self.expr.count("(") - self.expr.count(")"))
        try:
            out = subprocess.run([EXE, "deg" if self.degrees else "rad", expr],
                                 capture_output=True, text=True, timeout=5).stdout.strip()
        except FileNotFoundError:
            self.line2.config(text="Compile calc.cpp!", fg="#ff8787", font=("Consolas", 20, "bold"))
            return
        except Exception as ex:
            out = f"Error: {ex}"

        self.line1.config(text=self.pretty(expr) + " =")
        if out.startswith("Error"):
            self.line2.config(text=out[7:] or "Error", fg="#ff8787", font=("Consolas", 16, "bold"))
        else:
            self.last = out
            self.line2.config(text=out, fg="#63e6be",
                              font=("Consolas", 34 if len(out) < 12 else 22, "bold"))
            self.expr = out
        self.just_solved = True


if __name__ == "__main__":
    Calculator().mainloop()