import os

FRONTEND_DIR = os.path.join(os.path.dirname(__file__), '..', 'frontend')
OUTPUT_FILE = os.path.join(os.path.dirname(__file__), '..', 'src', 'UserControlGUI.cpp')

def read_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        return f.read()

def build_gui():
    print("Iniciando build del frontend MDE...")
    
    # 1. Leer index.html
    html_content = read_file(os.path.join(FRONTEND_DIR, 'index.html'))
    
    # 2. Leer e inyectar CSS
    css_content = read_file(os.path.join(FRONTEND_DIR, 'css', 'styles.css'))
    html_content = html_content.replace('<!-- INJECT_CSS -->', f'<style>\n{css_content}\n</style>')
    
    # 3. Leer e inyectar JS
    # El orden es crítico para la Causa Eficiente (dependencias lógicas)
    js_files = [
        'js/ui.js',          # Objeto window.ui base
        'js/websocket.js',   # Comunicación
        'js/joystick.js',    # Componente de control
        'js/trajectory.js',  # Componente planificador
        'js/app.js'          # Orquestador final
    ]
    
    js_combined = ""
    for js_file in js_files:
        js_combined += f"\n// --- {js_file} ---\n"
        js_combined += read_file(os.path.join(FRONTEND_DIR, js_file))
        js_combined += "\n"
        
    html_content = html_content.replace('<!-- INJECT_JS -->', f'<script>\n{js_combined}\n</script>')
    
    # 4. Generar el código C++ final
    cpp_template = f"""#include "UserControlGUI.h"

const char* HTML_PAGE = R"EOF(
{html_content}
)EOF";

const char* get_html_page() {{
    return HTML_PAGE;
}}
"""

    # 5. Escribir el archivo
    with open(OUTPUT_FILE, 'w', encoding='utf-8', newline='\n') as f:
        f.write(cpp_template)
        
    print(f"Build exitoso: {OUTPUT_FILE} generado correctamente.")

if __name__ == '__main__':
    build_gui()
