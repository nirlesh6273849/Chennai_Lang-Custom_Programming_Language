// Chennai Lang Online Compiler — Frontend Logic
const BASE_PATH = '/chennai_lang_onAWS';
const DEFAULT_CODE = `// Welcome to Chennai Lang! 🎉
// Click Run or press Ctrl+Enter to execute.

polamanna

main() {
    "Hello from Chennai Lang!" sollu

    int x;
    x = 10;
    int y;
    y = 20;
    int sum;
    sum = x + y;

    "x = " + x + ", y = " + y sollu
    "Sum = " + sum sollu

    if (sum > 25) {
        "Sum is greater than 25" sollu
    } else {
        "Sum is 25 or less" sollu
    }
}

niruthuanna
`;

let editor = null;
let isRunning = false;
let currentTheme = localStorage.getItem('chennai-theme') || 'dark';

// Apply saved theme
document.documentElement.setAttribute('data-theme', currentTheme);

// Monaco Editor Setup
require.config({
  paths: { vs: 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.45.0/min/vs' }
});

require(['vs/editor/editor.main'], function () {
  monaco.languages.register({ id: 'chennai-lang' });

  monaco.languages.setMonarchTokensProvider('chennai-lang', {
    keywords: ['polamanna','niruthuanna','Op','sollu','if','elif','else','while','main',
      'int','float','char','string','int[]','float[]','char[]','LHS','RHS','operator'],
    tokenizer: {
      root: [
        [/\/\/.*$/, 'comment'],
        [/"([^"\\]|\\.)*"/, 'string'],
        [/'([^'\\]|\\.)'/, 'string.char'],
        [/\d+\.\d+/, 'number.float'],
        [/\d+/, 'number'],
        [/[a-zA-Z_]\w*/, {
          cases: {
            'polamanna': 'keyword.flow', 'niruthuanna': 'keyword.flow',
            'main': 'keyword.function', 'sollu': 'keyword.print',
            'if|elif|else|while': 'keyword.control',
            'Op|LHS|RHS|operator': 'keyword.operator',
            'int|float|char|string': 'type', '@default': 'identifier'
          }
        }],
        [/[{}()\[\]]/, '@brackets'],
        [/:=/, 'operator'],
        [/[+\-*/%^=<>!&|]+/, 'operator'],
        [/[;,:]/, 'delimiter'],
      ]
    }
  });

  // Dark theme
  monaco.editor.defineTheme('chennai-dark', {
    base: 'vs-dark', inherit: true,
    rules: [
      { token: 'keyword.flow', foreground: '60a5fa', fontStyle: 'bold' },
      { token: 'keyword.function', foreground: 'a78bfa', fontStyle: 'bold' },
      { token: 'keyword.print', foreground: '67e8f9', fontStyle: 'bold' },
      { token: 'keyword.control', foreground: 'f472b6' },
      { token: 'keyword.operator', foreground: '93c5fd' },
      { token: 'type', foreground: '38bdf8' },
      { token: 'identifier', foreground: 'e2e8f0' },
      { token: 'number', foreground: 'a5f3fc' },
      { token: 'number.float', foreground: 'a5f3fc' },
      { token: 'string', foreground: '86efac' },
      { token: 'string.char', foreground: '86efac' },
      { token: 'comment', foreground: '475569', fontStyle: 'italic' },
      { token: 'operator', foreground: 'fbbf24' },
      { token: 'delimiter', foreground: '64748b' },
    ],
    colors: {
      'editor.background': '#0f1623',
      'editor.foreground': '#e2e8f0',
      'editor.lineHighlightBackground': '#1a223540',
      'editor.selectionBackground': '#3b82f630',
      'editorCursor.foreground': '#60a5fa',
      'editorLineNumber.foreground': '#334155',
      'editorLineNumber.activeForeground': '#64748b',
    }
  });

  // Light theme
  monaco.editor.defineTheme('chennai-light', {
    base: 'vs', inherit: true,
    rules: [
      { token: 'keyword.flow', foreground: '2563eb', fontStyle: 'bold' },
      { token: 'keyword.function', foreground: '7c3aed', fontStyle: 'bold' },
      { token: 'keyword.print', foreground: '0891b2', fontStyle: 'bold' },
      { token: 'keyword.control', foreground: 'db2777' },
      { token: 'keyword.operator', foreground: '2563eb' },
      { token: 'type', foreground: '0284c7' },
      { token: 'identifier', foreground: '1e293b' },
      { token: 'number', foreground: '0e7490' },
      { token: 'number.float', foreground: '0e7490' },
      { token: 'string', foreground: '16a34a' },
      { token: 'string.char', foreground: '16a34a' },
      { token: 'comment', foreground: '94a3b8', fontStyle: 'italic' },
      { token: 'operator', foreground: 'b45309' },
      { token: 'delimiter', foreground: '94a3b8' },
    ],
    colors: {
      'editor.background': '#ffffff',
      'editor.foreground': '#1e293b',
      'editor.lineHighlightBackground': '#f1f5f9',
      'editor.selectionBackground': '#3b82f625',
      'editorCursor.foreground': '#2563eb',
      'editorLineNumber.foreground': '#cbd5e1',
      'editorLineNumber.activeForeground': '#94a3b8',
    }
  });

  editor = monaco.editor.create(document.getElementById('editor-container'), {
    value: DEFAULT_CODE, language: 'chennai-lang',
    theme: currentTheme === 'dark' ? 'chennai-dark' : 'chennai-light',
    fontFamily: "'JetBrains Mono', 'Consolas', monospace",
    fontSize: 14, lineHeight: 24, padding: { top: 14, bottom: 14 },
    minimap: { enabled: false }, scrollBeyondLastLine: false, smoothScrolling: true,
    cursorBlinking: 'smooth', cursorSmoothCaretAnimation: 'on',
    bracketPairColorization: { enabled: true }, renderLineHighlight: 'all',
    automaticLayout: true, tabSize: 4, wordWrap: 'off',
    suggestOnTriggerCharacters: false, quickSuggestions: false,
    parameterHints: { enabled: false }, suggest: { enabled: false },
  });

  editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, runCode);
  loadExamples();
});

// Run Code
async function runCode() {
  if (isRunning || !editor) return;
  const code = editor.getValue().trim();
  if (!code) { showOutput('', 'No code to run.'); return; }
  setRunning(true);
  try {
    const res = await fetch(`${BASE_PATH}/api/run`, {
      method: 'POST', headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ code })
    });
    const data = await res.json();
    if (data.execTime !== undefined) document.getElementById('exec-time').textContent = `${data.execTime}ms`;
    showOutput(data.output || '', data.error || null);
  } catch (err) {
    showOutput('', 'Network error: Could not reach the server.');
  } finally { setRunning(false); }
}

function showOutput(stdout, error) {
  const el = document.getElementById('output-content');
  el.innerHTML = '';
  if (stdout) { const s = document.createElement('span'); s.className = 'output-success'; s.textContent = stdout; el.appendChild(s); }
  if (error) { const s = document.createElement('span'); s.className = 'output-error'; s.textContent = error; el.appendChild(s); }
  if (!stdout && !error) { const s = document.createElement('span'); s.className = 'output-placeholder'; s.textContent = 'Program produced no output.'; el.appendChild(s); }
  const dot = document.querySelector('.status-dot');
  const st = document.querySelector('.status-text');
  dot.className = 'status-dot' + (error ? ' error' : '');
  st.textContent = error ? 'Error' : 'Done';
}

function setRunning(running) {
  isRunning = running;
  const btn = document.getElementById('btn-run');
  const dot = document.querySelector('.status-dot');
  const st = document.querySelector('.status-text');
  if (running) { btn.classList.add('running'); btn.textContent = '⟳ Running...'; dot.className = 'status-dot running'; st.textContent = 'Running...'; }
  else { btn.classList.remove('running'); btn.textContent = 'Run'; }
}

// Examples
async function loadExamples() {
  try {
    const res = await fetch(`${BASE_PATH}/api/examples`);
    const data = await res.json();
    const dd = document.getElementById('example-dropdown');
    for (const [name, code] of Object.entries(data.examples)) {
      const opt = document.createElement('option');
      opt.value = name; opt.textContent = name.charAt(0).toUpperCase() + name.slice(1);
      opt.dataset.code = code; dd.appendChild(opt);
    }
    dd.addEventListener('change', (e) => {
      const sel = e.target.selectedOptions[0];
      if (sel && sel.dataset.code && editor) {
        editor.setValue(sel.dataset.code);
        document.getElementById('file-name').textContent = sel.value + '.ch';
      }
    });
  } catch (err) { console.warn('Could not load examples:', err); }
}

// Button Listeners
document.getElementById('btn-run').addEventListener('click', runCode);

document.getElementById('btn-clear').addEventListener('click', () => {
  if (editor) { editor.setValue(''); document.getElementById('file-name').textContent = 'untitled.ch'; document.getElementById('example-dropdown').value = ''; }
});

document.getElementById('btn-clear-output').addEventListener('click', () => {
  document.getElementById('output-content').innerHTML = '<span class="output-placeholder">Click <strong>Run</strong> or press <strong>Ctrl+Enter</strong> to execute your code.</span>';
  document.getElementById('exec-time').textContent = '';
  document.querySelector('.status-dot').className = 'status-dot';
  document.querySelector('.status-text').textContent = 'Ready';
});

// Syntax Sheet Toggle
function togglePanel(panelId, overlayId, btnId) {
  const panel = document.getElementById(panelId);
  const overlay = document.getElementById(overlayId);
  const btn = btnId ? document.getElementById(btnId) : null;
  const isOpen = panel.classList.contains('open');
  if (isOpen) { panel.classList.remove('open'); overlay.classList.remove('open'); if (btn) btn.classList.remove('active'); }
  else { panel.classList.add('open'); overlay.classList.add('open'); if (btn) btn.classList.add('active'); }
}

document.getElementById('btn-syntax').addEventListener('click', () => togglePanel('syntax-sheet', 'syntax-overlay', 'btn-syntax'));
document.getElementById('btn-close-syntax').addEventListener('click', () => togglePanel('syntax-sheet', 'syntax-overlay', 'btn-syntax'));
document.getElementById('syntax-overlay').addEventListener('click', () => togglePanel('syntax-sheet', 'syntax-overlay', 'btn-syntax'));

// How It Works Modal
document.getElementById('btn-how').addEventListener('click', () => togglePanel('how-modal', 'how-overlay', 'btn-how'));
document.getElementById('btn-close-how').addEventListener('click', () => togglePanel('how-modal', 'how-overlay', 'btn-how'));
document.getElementById('how-overlay').addEventListener('click', () => togglePanel('how-modal', 'how-overlay', 'btn-how'));

// Theme Toggle
document.getElementById('btn-theme').addEventListener('click', () => {
  currentTheme = currentTheme === 'dark' ? 'light' : 'dark';
  document.documentElement.setAttribute('data-theme', currentTheme);
  localStorage.setItem('chennai-theme', currentTheme);
  if (editor) monaco.editor.setTheme(currentTheme === 'dark' ? 'chennai-dark' : 'chennai-light');
});

// Fullscreen Toggle
document.getElementById('btn-fullscreen').addEventListener('click', () => {
  document.body.classList.toggle('fullscreen');
  if (editor) setTimeout(() => editor.layout(), 100);
});

// Escape key closes modals
document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape') {
    if (document.getElementById('syntax-sheet').classList.contains('open')) togglePanel('syntax-sheet', 'syntax-overlay', 'btn-syntax');
    if (document.getElementById('how-modal').classList.contains('open')) togglePanel('how-modal', 'how-overlay', 'btn-how');
  }
});
