import { spawn } from 'child_process';
import { readdir, readFile, writeFile, mkdir } from 'fs/promises';
import { exec } from 'child_process';
import { promisify } from 'util';
import path from 'path';
import { existsSync } from 'fs';

const execAsync = promisify(exec);

// Configuration - adjusted for Linux environment
const EXECUTABLE = './main'; // Linux executable without .exe extension
const TEST_DIR = './test_cases';
const OUTPUT_DIR = './my_outputs';

async function ensureOutputDir() {
  try {
    if (!existsSync(OUTPUT_DIR)) {
      await mkdir(OUTPUT_DIR);
      console.log(`Created output directory: ${OUTPUT_DIR}`);
    }
  } catch (error) {
    console.error('Error creating output directory:', error);
  }
}

async function runTest(testFile) {
  const testName = path.basename(testFile, '.in');
  const outputFile = path.join(OUTPUT_DIR, `${testName}.out`);
  const expectedFile = path.join(TEST_DIR, `${testName}.out`);
  
  console.log(`Running test: ${testName}`);
  
  return new Promise((resolve, reject) => {
    // Run the program with input from test file
    const process = spawn(EXECUTABLE, [], { shell: true });
    
    // Capture stdout
    let output = '';
    process.stdout.on('data', (data) => {
      output += data.toString();
    });
    
    // Handle errors
    process.stderr.on('data', (data) => {
      // Ignore stderr as it contains prompts
    });
    
    // Read test input file and feed it to the process
    readFile(testFile, 'utf8')
      .then(inputData => {
        process.stdin.write(inputData);
        process.stdin.end();
      })
      .catch(err => reject(err));
    
    // When the process completes
    process.on('close', async (code) => {
      if (code !== 0) {
        reject(new Error(`Process exited with code ${code}`));
        return;
      }
      
      try {
        // Save output to file
        await writeFile(outputFile, output);
        
        // Compare with expected output using diff instead of fc
        const { stdout, stderr } = await execAsync(`diff -u ${outputFile} ${expectedFile}`);
        
        if (stderr) {
          console.log(`❌ Test ${testName} FAILED: ${stderr}`);
          resolve({ testName, passed: false, error: stderr });
        } else if (!stdout) {
          // Empty stdout means no differences
          console.log(`✅ Test ${testName} PASSED`);
          resolve({ testName, passed: true });
        } else {
          console.log(`❌ Test ${testName} FAILED: Output differs from expected`);
          console.log(stdout);
          resolve({ testName, passed: false, differences: stdout });
        }
      } catch (error) {
        // If diff command returns non-zero, it means files are different
        if (error.code === 1) {
          console.log(`❌ Test ${testName} FAILED: Files differ`);
          console.log(error.stdout);
          resolve({ testName, passed: false, differences: error.stdout });
        } else {
          console.log(`❌ Test ${testName} ERROR: ${error.message}`);
          resolve({ testName, passed: false, error: error.message });
        }
      }
    });
  });
}

async function main() {
  try {
    await ensureOutputDir();
    
    // Find all test input files
    const files = await readdir(TEST_DIR);
    const testFiles = files
      .filter(file => file.endsWith('.in'))
      .map(file => path.join(TEST_DIR, file));
    
    console.log(`Found ${testFiles.length} test files`);
    
    // Run all tests
    const results = await Promise.all(testFiles.map(runTest));
    
    // Summarize results
    const passedTests = results.filter(r => r.passed).length;
    console.log('\n===== TEST SUMMARY =====');
    console.log(`Passed: ${passedTests}/${testFiles.length}`);
    
    if (passedTests === testFiles.length) {
      console.log('🎉 All tests passed!');
    } else {
      console.log('❌ Some tests failed. Check output above for details.');
    }
  } catch (error) {
    console.error('Error running tests:', error);
  }
}

main();
