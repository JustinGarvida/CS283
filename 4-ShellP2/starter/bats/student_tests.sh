#!/usr/bin/env bats

# File: student_tests.sh

@test "Built-in command: exit terminates shell" {
    run ./dsh <<EOF
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Built-in command: dragon prints dragon" {
    run ./dsh <<EOF
dragon
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"dragon"* ]]
}

@test "Built-in command: cd changes directory" {
    run ./dsh <<EOF
cd /
pwd
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"/"* ]]
}

@test "External command: ls executes successfully" {
    run ./dsh <<EOF
ls
exit
EOF
    [ "$status" -eq 0 ]
    [ "${lines[0]}" != "" ]
}

@test "External command with arguments: ls -l executes successfully" {
    run ./dsh <<EOF
ls -l
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"total"* ]]
}

@test "External command: invalid command returns error" {
    run ./dsh <<EOF
nonexistentcommand
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"No such file or directory"* || "$output" == "" ]]
}

@test "Empty input: should prompt again" {
    run ./dsh <<EOF

exit
EOF
    [ "$status" -eq 0 ]
}

@test "Whitespace input: should prompt again" {
    run ./dsh <<EOF
    
exit
EOF
    [ "$status" -eq 0 ]
}

@test "Complex command: echo with quotes" {
    run ./dsh <<EOF
echo "Hello World"
exit
EOF
    [ "$status" -eq 0 ]
    [[ "$output" == *"Hello World"* ]]
}

@test "Multiple commands executed sequentially" {
    run ./dsh <<EOF
pwd
cd ..
pwd
exit
EOF
    [ "$status" -eq 0 ]
    [ "${#lines[@]}" -ge 2 ]
    [ "${lines[0]}" != "${lines[1]}" ]
}
