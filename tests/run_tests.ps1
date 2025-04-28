# Test script for Filinator on Windows (PowerShell version)
# Tests encoding and decoding of file and directory names

# Colors for better readability
$GREEN = "Green"
$RED = "Red"
$YELLOW = "Yellow"
$RESET = "White"

function Write-ColorOutput($ForegroundColor) {
    $fc = $host.UI.RawUI.ForegroundColor
    $host.UI.RawUI.ForegroundColor = $ForegroundColor
    if ($args) {
        Write-Output $args
    }
    else {
        $input | Write-Output
    }
    $host.UI.RawUI.ForegroundColor = $fc
}

Write-ColorOutput $YELLOW "=== Filinator3000 PowerShell Test ==="

# Project root directory
$PROJECT_DIR = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Write-Output "Project directory: $PROJECT_DIR"

# Path to the Filinator executable
$FILINATOR = Join-Path -Path $PROJECT_DIR -ChildPath "filinator.exe"

# Check that Filinator exists
if (-not (Test-Path -Path $FILINATOR)) {
    Write-ColorOutput $RED "Error: $FILINATOR does not exist. Compile the project first."
    exit 1
}

# Create the test structure
$TEST_BASE = Join-Path -Path $PROJECT_DIR -ChildPath "tests\test_data"
if (Test-Path -Path $TEST_BASE) {
    Remove-Item -Path $TEST_BASE -Recurse -Force
}
New-Item -Path $TEST_BASE -ItemType Directory -Force | Out-Null

Write-ColorOutput $YELLOW "1. Creating a test directory and file structure..."

# Create directories with spaces and special characters
New-Item -Path "$TEST_BASE\Normal Directory" -ItemType Directory -Force | Out-Null
New-Item -Path "$TEST_BASE\Directory with spaces" -ItemType Directory -Force | Out-Null
New-Item -Path "$TEST_BASE\Directory_without_spaces" -ItemType Directory -Force | Out-Null
New-Item -Path "$TEST_BASE\Directory with spaces\Subdirectory" -ItemType Directory -Force | Out-Null
New-Item -Path "$TEST_BASE\Directory with § special character" -ItemType Directory -Force | Out-Null

# Create deep nested directories
New-Item -Path "$TEST_BASE\Nested\Folder\Structure\With\Many\Levels" -ItemType Directory -Force | Out-Null
New-Item -Path "$TEST_BASE\Another Nested\Structure With\Spaces in path" -ItemType Directory -Force | Out-Null

# Create files with different names
"Normal content" | Out-File -FilePath "$TEST_BASE\normal_file.txt" -Encoding utf8
"Content with spaces" | Out-File -FilePath "$TEST_BASE\file with spaces.txt" -Encoding utf8

# Create directories and files with slash in name
New-Item -Path "$TEST_BASE\file\with" -ItemType Directory -Force | Out-Null
"Content with slash" | Out-File -FilePath "$TEST_BASE\file\with\slash.txt" -Encoding utf8

# File with section character
"Content with section" | Out-File -FilePath "$TEST_BASE\file§with§section.txt" -Encoding utf8

# Files in subdirectories
"Content in subdirectory" | Out-File -FilePath "$TEST_BASE\Directory with spaces\file in directory.txt" -Encoding utf8
"Special content" | Out-File -FilePath "$TEST_BASE\Directory with § special character\special file.txt" -Encoding utf8

# Create deeply nested file
New-Item -Path "$TEST_BASE\Directory with spaces\file\with" -ItemType Directory -Force | Out-Null
"Content with spaces and slashes" | Out-File -FilePath "$TEST_BASE\Directory with spaces\file\with\paths.txt" -Encoding utf8

# Add some files in nested directories
"Nested content 1" | Out-File -FilePath "$TEST_BASE\Nested\Folder\Structure\With\Many\Levels\deep_file.txt" -Encoding utf8
"Nested content 2" | Out-File -FilePath "$TEST_BASE\Nested\Folder\Structure\With\Many\Levels\another deep file.txt" -Encoding utf8
"Nested content 3" | Out-File -FilePath "$TEST_BASE\Another Nested\Structure With\Spaces in path\nested file with spaces.txt" -Encoding utf8

# List the created structure
Write-ColorOutput $YELLOW "Test structure created:"
Get-ChildItem -Path $TEST_BASE -Recurse | Sort-Object FullName | ForEach-Object { $_.FullName }

# Save the file list for later comparison
$original_files = Get-ChildItem -Path $TEST_BASE -File -Recurse | Sort-Object FullName | ForEach-Object { $_.FullName }
$original_dirs = Get-ChildItem -Path $TEST_BASE -Directory -Recurse | Sort-Object FullName | ForEach-Object { $_.FullName }

$original_files | Out-File -FilePath "$PROJECT_DIR\tests\original_files.txt" -Encoding utf8
$original_dirs | Out-File -FilePath "$PROJECT_DIR\tests\original_dirs.txt" -Encoding utf8

# Calculate checksums for files
Write-ColorOutput $YELLOW "Calculating checksums of original files..."
$original_checksums = @()

foreach ($file in $original_files) {
    $hash = Get-FileHash -Path $file -Algorithm SHA256
    $original_checksums += "$($hash.Hash) $file"
}

$original_checksums | Out-File -FilePath "$PROJECT_DIR\tests\original_checksums.txt" -Encoding utf8

# Encoding phase
Write-ColorOutput $YELLOW "2. Encoding the structure..."
& $FILINATOR -encode $TEST_BASE -output "$PROJECT_DIR\tests\encoded"

# Check that the encoded directory exists
if (-not (Test-Path -Path "$PROJECT_DIR\tests\encoded")) {
    Write-ColorOutput $RED "Error: Encoding failed, encoded directory does not exist."
    exit 1
}

Write-ColorOutput $YELLOW "Content of the encoded directory:"
$encoded_files = Get-ChildItem -Path "$PROJECT_DIR\tests\encoded" -File -Recurse | Sort-Object FullName | ForEach-Object { $_.FullName }
$encoded_files

# Calculate checksums for encoded files
Write-ColorOutput $YELLOW "Calculating checksums of encoded files..."
$encoded_checksums = @()

foreach ($file in $encoded_files) {
    $hash = Get-FileHash -Path $file -Algorithm SHA256
    $encoded_checksums += "$($hash.Hash) $file"
}

$encoded_checksums | Out-File -FilePath "$PROJECT_DIR\tests\encoded_checksums.txt" -Encoding utf8

# Check that all files have been encoded
$ORIGINAL_COUNT = $original_files.Count
$ENCODED_COUNT = $encoded_files.Count

if ($ORIGINAL_COUNT -ne $ENCODED_COUNT) {
    Write-ColorOutput $RED "Error: Different number of files after encoding."
    Write-ColorOutput $RED "Original: $ORIGINAL_COUNT, Encoded: $ENCODED_COUNT"
    exit 1
}

Write-ColorOutput $GREEN "✓ All files have been encoded."

# Create a copy of the encoded structure for decoding
Write-ColorOutput $YELLOW "3. Preparing for decoding..."
if (Test-Path -Path "$PROJECT_DIR\tests\to_decode") {
    Remove-Item -Path "$PROJECT_DIR\tests\to_decode" -Recurse -Force
}
Copy-Item -Path "$PROJECT_DIR\tests\encoded" -Destination "$PROJECT_DIR\tests\to_decode" -Recurse

# List the structure to decode
Write-ColorOutput $YELLOW "Structure to decode:"
Get-ChildItem -Path "$PROJECT_DIR\tests\to_decode" -File -Recurse | Sort-Object FullName | ForEach-Object { $_.FullName }

# Decoding phase
Write-ColorOutput $YELLOW "4. Decoding the structure..."
& $FILINATOR -decode "$PROJECT_DIR\tests\to_decode"

# Check the decoded structure
Write-ColorOutput $YELLOW "Structure after decoding:"
$decoded_files = Get-ChildItem -Path "$PROJECT_DIR\tests\to_decode" -File -Recurse | Sort-Object FullName | ForEach-Object { $_.FullName }
$decoded_dirs = Get-ChildItem -Path "$PROJECT_DIR\tests\to_decode" -Directory -Recurse | Sort-Object FullName | ForEach-Object { $_.FullName }

$decoded_files | Out-File -FilePath "$PROJECT_DIR\tests\decoded_files.txt" -Encoding utf8
$decoded_dirs | Out-File -FilePath "$PROJECT_DIR\tests\decoded_dirs.txt" -Encoding utf8
$decoded_files

# Calculate checksums for decoded files
Write-ColorOutput $YELLOW "Calculating checksums of decoded files..."
$decoded_checksums = @()

foreach ($file in $decoded_files) {
    $hash = Get-FileHash -Path $file -Algorithm SHA256
    $decoded_checksums += "$($hash.Hash) $file"
}

$decoded_checksums | Out-File -FilePath "$PROJECT_DIR\tests\decoded_checksums.txt" -Encoding utf8

# Check that decoding created a structure similar to the original
$DECODED_COUNT = $decoded_files.Count

if ($ORIGINAL_COUNT -ne $DECODED_COUNT) {
    Write-ColorOutput $RED "Error: Different number of files after decoding."
    Write-ColorOutput $RED "Original: $ORIGINAL_COUNT, Decoded: $DECODED_COUNT"
    exit 1
}

Write-ColorOutput $GREEN "✓ All files have been decoded."

# Check that file content is preserved
$ORIG_CHECKSUM_COUNT = (Get-Content -Path "$PROJECT_DIR\tests\original_checksums.txt").Count
$DECODED_CHECKSUM_COUNT = (Get-Content -Path "$PROJECT_DIR\tests\decoded_checksums.txt").Count

if ($ORIG_CHECKSUM_COUNT -ne $DECODED_CHECKSUM_COUNT) {
    Write-ColorOutput $RED "Error: Different number of checksums."
    exit 1
}

# Compare total file sizes as a basic integrity check
Write-ColorOutput $YELLOW "Comparing total file sizes..."

$ORIG_TOTAL_SIZE = (Get-ChildItem -Recurse -File $TEST_BASE | Measure-Object -Property Length -Sum).Sum
$DECODED_TOTAL_SIZE = (Get-ChildItem -Recurse -File "$PROJECT_DIR\tests\to_decode" | Measure-Object -Property Length -Sum).Sum

if ($ORIG_TOTAL_SIZE -ne $DECODED_TOTAL_SIZE) {
    Write-ColorOutput $RED "Error: Different total data size after decoding."
    Write-ColorOutput $RED "Original: $ORIG_TOTAL_SIZE, Decoded: $DECODED_TOTAL_SIZE"
    exit 1
}

Write-ColorOutput $GREEN "✓ Data sizes are identical."
Write-ColorOutput $GREEN "=== All tests passed! ==="

exit 0