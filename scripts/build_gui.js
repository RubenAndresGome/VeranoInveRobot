const fs = require('fs');
const path = require('path');

const FRONTEND_DIR = path.join(__dirname, '..', 'frontend');
const OUTPUT_FILE = path.join(__dirname, '..', 'src', 'UserControlGUI.cpp');

function readFile(filepath) {
    return fs.readFileSync(filepath, 'utf8');
}

function buildGui() {
    console.log("Iniciando build del frontend MDE...");
    
    // 1. Leer index.html
    let htmlContent = readFile(path.join(FRONTEND_DIR, 'index.html'));
    
    // 2. Leer e inyectar CSS
    const cssContent = readFile(path.join(FRONTEND_DIR, 'css', 'styles.css'));
    htmlContent = htmlContent.replace('<!-- INJECT_CSS -->', `<style>\n${cssContent}\n</style>`);
    
    // 3. Leer e inyectar JS
    const jsFiles = [
        'js/ui.js',
        'js/websocket.js',
        'js/joystick.js',
        'js/trajectory.js',
        'js/app.js'
    ];
    
    let jsCombined = "";
    for (const jsFile of jsFiles) {
        jsCombined += `\n// --- ${jsFile} ---\n`;
        jsCombined += readFile(path.join(FRONTEND_DIR, jsFile));
        jsCombined += "\n";
    }
        
    htmlContent = htmlContent.replace('<!-- INJECT_JS -->', `<script>\n${jsCombined}\n</script>`);
    
    // 4. Generar el código C++ final
    const cppTemplate = `#include "UserControlGUI.h"\n\nconst char* HTML_PAGE = R"EOF(\n${htmlContent}\n)EOF";\n\nconst char* get_html_page() {\n    return HTML_PAGE;\n}\n`;

    // 5. Escribir el archivo
    fs.writeFileSync(OUTPUT_FILE, cppTemplate, 'utf8');
        
    console.log(`Build exitoso: ${OUTPUT_FILE} generado correctamente.`);
}

buildGui();
