vim.keymap.set("n", "<F5>", "<cmd>!./bin/main main.lox<CR>", { desc = "Run the app" })

vim.keymap.set("n", "<leader>bb", "<cmd>!bear -- make<CR>", { desc = "Build" })
vim.keymap.set("n", "<leader>br", "<cmd>!make clean && make release<CR>", { desc = "Release build" })
vim.keymap.set("n", "<leader>bc", "<cmd>!make clean<CR>", { desc = "Clean" })
