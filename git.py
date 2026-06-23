import subprocess
import time
import sys
import os

# ── CONFIG ──────────────────────────────────────────────────────────────────
REPO_PATH   = r"C:\Users\Mxzzy\Downloads\MISERABLE"
BRANCH      = "main"          # cambia si es necesario
COMMIT_MSG  = "chore: auto-update"
INTERVAL    = 10               # segundos entre cada check
# ────────────────────────────────────────────────────────────────────────────

def run(cmd, cwd=REPO_PATH):
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, shell=True)
    return result.stdout.strip(), result.stderr.strip(), result.returncode

def hay_cambios():
    out, _, _ = run("git status --porcelain")
    return bool(out)

def push():
    print("[*] Cambios detectados, subiendo...")

    _, err, code = run("git add -A")
    if code != 0:
        print(f"[!] git add falló: {err}"); return

    _, err, code = run(f'git commit -m "{COMMIT_MSG}"')
    if code != 0:
        print(f"[!] git commit falló: {err}"); return

    out, err, code = run(f"git push origin {BRANCH}")
    if code != 0:
        print(f"[!] git push falló: {err}")
    else:
        print(f"[+] Push exitoso")

print(f"[*] Watching {REPO_PATH} cada {INTERVAL}s... (Ctrl+C para salir)")
try:
    while True:
        if hay_cambios():
            push()
        else:
            print("[~] Sin cambios", end="\r")
        time.sleep(INTERVAL)
except KeyboardInterrupt:
    print("\n[*] Detenido.")