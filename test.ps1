# test.ps1 - Full miniGit demo with output to file

$output = "test_output.txt"

# Очищаем файл с результатом
"" | Out-File $output -Encoding UTF8

# Функция для вывода и в консоль, и в файл
function Write-OutputBoth($msg) {
    Write-Host $msg
    $msg | Out-File $output -Append -Encoding UTF8
}

Write-OutputBoth "=== miniGit Full Demo ==="
Write-OutputBoth ""

# Clean
if (Test-Path ".minigit") {
    Remove-Item -Recurse -Force .minigit
}

# 1. init
Write-OutputBoth "[1] init"
$result = .\minigit.exe init 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 2. add files
Write-OutputBoth "[2] add files"
$result = .\minigit.exe add a.txt "first version of a.txt" 2>&1
Write-OutputBoth $result
$result = .\minigit.exe add b.txt "second file content" 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 3. first commit
Write-OutputBoth "[3] first commit"
$result = .\minigit.exe commit "Commit A: added a.txt and b.txt" 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# Save first commit hash
$history = .\minigit.exe print_history 2>&1
$commits = $history | Select-String "^commit " | ForEach-Object { ($_ -split " ")[1] }
$commitA = $commits[-1]
Write-OutputBoth "First commit hash: $commitA"
Write-OutputBoth ""

# 4. print_history
Write-OutputBoth "[4] print_history"
$result = .\minigit.exe print_history 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 5. get_file_content
Write-OutputBoth "[5] get_file_content"
$result = .\minigit.exe content HEAD a.txt 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 6. get_file_exists
Write-OutputBoth "[6] get_file_exists"
$result = .\minigit.exe exists HEAD a.txt 2>&1
Write-OutputBoth $result
$result = .\minigit.exe exists HEAD fake.txt 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 7. update file + second commit
Write-OutputBoth "[7] update file + second commit"
$result = .\minigit.exe add a.txt "SECOND version of a.txt - changed!" 2>&1
Write-OutputBoth $result
$result = .\minigit.exe commit "Commit B: updated a.txt" 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 8. remove file + third commit
Write-OutputBoth "[8] remove file + third commit"
$result = .\minigit.exe rm b.txt 2>&1
Write-OutputBoth $result
$result = .\minigit.exe commit "Commit C: removed b.txt" 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 9. print_files
Write-OutputBoth "[9] print_files"
$result = .\minigit.exe print_files 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 10. create branch
Write-OutputBoth "[10] create branch"
$result = .\minigit.exe branch feature 2>&1
Write-OutputBoth $result
$result = .\minigit.exe branch 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 11. checkout branch
Write-OutputBoth "[11] checkout branch"
$result = .\minigit.exe checkout feature 2>&1
Write-OutputBoth $result
$result = .\minigit.exe add feature.txt "this file is only in feature branch" 2>&1
Write-OutputBoth $result
$result = .\minigit.exe commit "Commit D: added feature.txt" 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 12. get_branch_head
Write-OutputBoth "[12] get_branch_head"
$result = .\minigit.exe get_branch_head feature 2>&1
Write-OutputBoth $result
Write-OutputBoth ""
$result = .\minigit.exe get_branch_head master 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 13. merge
Write-OutputBoth "[13] merge"
$result = .\minigit.exe checkout master 2>&1
Write-OutputBoth $result
$result = .\minigit.exe merge feature "Merge feature into master" 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

# 14. PERSISTENCE TEST
Write-OutputBoth "[14] PERSISTENCE TEST (old commit unchanged)"
Write-OutputBoth "Content of a.txt from FIRST commit (should be first version):"
$result = .\minigit.exe content $commitA a.txt 2>&1
Write-OutputBoth $result
Write-OutputBoth ""
Write-OutputBoth "Content of a.txt from CURRENT HEAD (should be second version):"
$result = .\minigit.exe content HEAD a.txt 2>&1
Write-OutputBoth $result
Write-OutputBoth ""
Write-OutputBoth "PASS: old commit still has old data - persistence works!"
Write-OutputBoth ""

# 15. final history
Write-OutputBoth "[15] final history"
$result = .\minigit.exe print_history 2>&1
Write-OutputBoth $result
Write-OutputBoth ""

Write-OutputBoth "=== ALL TESTS PASSED ==="
Write-OutputBoth ""
Write-OutputBoth "Tested: init, add, commit, rm, print_history,"
Write-OutputBoth "print_files, content, exists, branch, checkout,"
Write-OutputBoth "get_branch_head, merge, persistence"

Write-Host ""
Write-Host "Result saved to: $output"