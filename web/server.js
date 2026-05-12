const express = require('express');
const { execFile } = require('child_process');
const fs = require('fs');
const path = require('path');
const os = require('os');
const { v4: uuidv4 } = require('uuid');

const app = express();
const PORT = process.env.PORT || 3000;
const BASE_PATH = '/chennai_lang_onAWS';

// Path to the Chennai Lang binary
const BINARY_PATH = process.env.CHENNAI_LANG_BIN || path.join(__dirname, '..', 'chennai_lang.exe');

// Security limits
const TIMEOUT_MS = 5000;        // 5 second execution timeout
const MAX_OUTPUT_BYTES = 50000; // 50KB max output
const MAX_CODE_BYTES = 50000;   // 50KB max code size

// Middleware
app.use(express.json({ limit: '100kb' }));

// Serve static files at the base path
app.use(BASE_PATH, express.static(path.join(__dirname, 'public')));

// Redirect base path without trailing slash
app.get(BASE_PATH, (req, res) => {
  res.redirect(BASE_PATH + '/');
});

// Example programs endpoint
app.get(`${BASE_PATH}/api/examples`, (req, res) => {
  const examplesDir = path.join(__dirname, '..', 'examples');
  try {
    const files = fs.readdirSync(examplesDir).filter(f => f.endsWith('.ch'));
    const examples = {};
    for (const file of files) {
      const name = path.basename(file, '.ch');
      examples[name] = fs.readFileSync(path.join(examplesDir, file), 'utf-8');
    }
    res.json({ examples });
  } catch (err) {
    res.json({ examples: {} });
  }
});

// Run code endpoint
app.post(`${BASE_PATH}/api/run`, (req, res) => {
  const { code } = req.body;

  if (!code || typeof code !== 'string') {
    return res.status(400).json({ error: 'No code provided' });
  }

  if (code.length > MAX_CODE_BYTES) {
    return res.status(400).json({ error: 'Code too large (max 50KB)' });
  }

  // Create a unique temp file
  const tempDir = os.tmpdir();
  const filename = `chennai_${uuidv4()}.ch`;
  const tempFile = path.join(tempDir, filename);

  try {
    fs.writeFileSync(tempFile, code, 'utf-8');
  } catch (err) {
    return res.status(500).json({ error: 'Failed to create temp file' });
  }

  const startTime = Date.now();

  execFile(BINARY_PATH, [tempFile], { timeout: TIMEOUT_MS, maxBuffer: MAX_OUTPUT_BYTES }, (error, stdout, stderr) => {
    // Clean up temp file
    try { fs.unlinkSync(tempFile); } catch (e) { /* ignore */ }

    const execTime = Date.now() - startTime;

    if (error) {
      if (error.killed) {
        return res.json({
          output: stdout || '',
          error: 'Execution timed out (5 second limit)',
          execTime
        });
      }
      return res.json({
        output: stdout || '',
        error: stderr || error.message || 'Execution failed',
        execTime
      });
    }

    res.json({
      output: stdout || '',
      error: stderr || null,
      execTime
    });
  });
});

// Download binary endpoint
app.get(`${BASE_PATH}/api/download`, (req, res) => {
  if (fs.existsSync(BINARY_PATH)) {
    res.download(BINARY_PATH, 'chennai_lang.exe');
  } else {
    res.status(404).json({ error: 'Binary not available for download' });
  }
});

// Health check
app.get(`${BASE_PATH}/api/health`, (req, res) => {
  res.json({ status: 'ok', binary: fs.existsSync(BINARY_PATH) });
});

app.listen(PORT, () => {
  console.log(`\n  Chennai Lang Online Compiler`);
  console.log(`  Running at: http://localhost:${PORT}${BASE_PATH}/`);
  console.log(`  Binary: ${BINARY_PATH}`);
  console.log(`  Binary exists: ${fs.existsSync(BINARY_PATH)}\n`);
});
