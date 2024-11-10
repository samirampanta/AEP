#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  
#include <windows.h> 
#include <locale.h> 

#define MAX_LOGIN 50
#define MAX_SENHA 20
#define SHIFT 3

#define CLEAN_BUFF do{ int c; while((c = getchar()) != '\n' && c != EOF);}while(0)

void limparTela() {
    system("cls");
}

void sleep_seconds(int seconds) {
    #ifdef _WIN32
        Sleep(seconds * 1000);
    #else
        sleep(seconds);
    #endif
}

void encrypt(char *senha, int shift) {
    for (int i = 0; senha[i] != '\0'; i++) {
        if (senha[i] >= 'A' && senha[i] <= 'Z') {
            senha[i] = ((senha[i] - 'A' + shift) % 26) + 'A';
        } else if (senha[i] >= 'a' && senha[i] <= 'z') {
            senha[i] = ((senha[i] - 'a' + shift) % 26) + 'a';
        } else if (senha[i] >= '0' && senha[i] <= '9') {
            senha[i] = ((senha[i] - '0' + shift) % 10) + '0';
        }
    }
}

void decrypt(char *senha, int shift) {
    for (int i = 0; senha[i] != '\0'; i++) {
        if (senha[i] >= 'A' && senha[i] <= 'Z') {
            senha[i] = ((senha[i] - 'A' - shift + 26) % 26) + 'A';
        } else if (senha[i] >= 'a' && senha[i] <= 'z') {
            senha[i] = ((senha[i] - 'a' - shift + 26) % 26) + 'a';
        } else if (senha[i] >= '0' && senha[i] <= '9') {
            senha[i] = ((senha[i] - '0' - shift + 10) % 10) + '0';
        }
    }
}

void listarUsuarios() {
    FILE *fp = fopen("usuarios.txt", "r");
    if (!fp) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }

    char login[MAX_LOGIN], senha[MAX_SENHA];
    printf("Usuarios cadastrados:\n");
    while (fscanf(fp, "%s %s", login, senha) != EOF) {
        printf("Usuario: %s\n", login);
    }

    fclose(fp);

    printf("\nPressione qualquer tecla para voltar ao menu...\n");
    getchar();
}

int buscarUsuario(FILE *fp, const char *user) {
    char login[MAX_LOGIN], senha[MAX_SENHA];
    while (fscanf(fp, "%s %s", login, senha) != EOF) {
        if (strcmp(login, user) == 0) {
            return 1;
        }
    }
    return 0;
}

int loginUsuario(const char *user, const char *senha) {
    FILE *fp = fopen("usuarios.txt", "r");
    if (!fp) {
        printf("Erro ao abrir o arquivo!\n");
        return 0;
    }

    char login[MAX_LOGIN], storedSenha[MAX_SENHA];
    while (fscanf(fp, "%s %s", login, storedSenha) != EOF) {
        if (strcmp(login, user) == 0) {
            decrypt(storedSenha, SHIFT);
            if (strcmp(storedSenha, senha) == 0) {
                fclose(fp);
                return 1;
            }
        }
    }

    fclose(fp);
    return 0;
}

void adicionarUsuario() {
    char login[MAX_LOGIN], senha[MAX_SENHA];

    printf("Digite o nome de usuario: ");
    fgets(login, MAX_LOGIN, stdin);
    login[strcspn(login, "\n")] = 0;

    FILE *fp = fopen("usuarios.txt", "a+");
    if (!fp) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }

    if (buscarUsuario(fp, login)) {
        printf("Usuário já existe!\n");
        fclose(fp);
        return;
    }

    printf("Digite a senha: ");
    fgets(senha, MAX_SENHA, stdin);
    senha[strcspn(senha, "\n")] = 0;
    encrypt(senha, SHIFT);

    fprintf(fp, "%s %s\n", login, senha);
    printf("Usuário adicionado com sucesso!\n");

    fclose(fp);
}

void alterarUsuario() {
    char login[MAX_LOGIN], novaSenha[MAX_SENHA];
    FILE *fp = fopen("usuarios.txt", "r+");
    if (!fp) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }

    printf("Digite o nome do usuario para alteração: ");
    fgets(login, MAX_LOGIN, stdin);
    login[strcspn(login, "\n")] = 0;

    char tempLogin[MAX_LOGIN], tempSenha[MAX_SENHA];
    long pos;
    int encontrado = 0;
    while (fscanf(fp, "%s %s", tempLogin, tempSenha) != EOF) {
        pos = ftell(fp) - strlen(tempLogin) - strlen(tempSenha) - 2;
        if (strcmp(tempLogin, login) == 0) {
            printf("Digite a nova senha: ");
            fgets(novaSenha, MAX_SENHA, stdin);
            novaSenha[strcspn(novaSenha, "\n")] = 0;

            encrypt(novaSenha, SHIFT);

            fseek(fp, pos, SEEK_SET);
            fprintf(fp, "%s %s\n", login, novaSenha);
            printf("Senha alterada com sucesso!\n");

            encontrado = 1;
            break;
        }
    }

    if (!encontrado) {
        printf("Usuário não encontrado!\n");
    }

    fclose(fp);
}

void excluirUsuario() {
    char login[MAX_LOGIN];
    FILE *fp = fopen("usuarios.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        printf("Erro ao abrir o arquivo!\n");
        return;
    }

    printf("Digite o nome do usuario para exclusao: ");
    fgets(login, MAX_LOGIN, stdin);
    login[strcspn(login, "\n")] = 0;

    char tempLogin[MAX_LOGIN], tempSenha[MAX_SENHA];
    int excluido = 0;
    while (fscanf(fp, "%s %s", tempLogin, tempSenha) != EOF) {
        if (strcmp(tempLogin, login) != 0) {
            fprintf(temp, "%s %s\n", tempLogin, tempSenha);
        } else {
            excluido = 1;
        }
    }

    fclose(fp);
    fclose(temp);

    remove("usuarios.txt");
    rename("temp.txt", "usuarios.txt");

    if (excluido) {
        printf("Usuário excluído com sucesso!\n");
    } else {
        printf("Usuário não encontrado!\n");
    }
}

int main() {
    int opcao;
    char user[MAX_LOGIN], senha[MAX_SENHA];

    int logado = 0;

    setlocale(LC_ALL, "Portuguese");

    do {
        limparTela();

        if (logado) {
            printf("Bem-vindo(a), %s!\n\n", user);
        } else {
            printf("Bem-vindo ao sistema de gerenciamento de usuários!\n\n");
        }

        printf("1. Cadastro\n");
        printf("2. Login\n");
        printf("3. Alterar usuario\n");
        printf("4. Excluir usuario\n");
        printf("5. Listar usuarios\n");
        printf("6. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);
        CLEAN_BUFF;

        switch (opcao) {
            case 1:
                adicionarUsuario();
                break;

            case 2:
                printf("Digite o nome de usuario: ");
                fgets(user, MAX_LOGIN, stdin);
                user[strcspn(user, "\n")] = 0;

                printf("Digite a senha: ");
                fgets(senha, MAX_SENHA, stdin);
                senha[strcspn(senha, "\n")] = 0;

                if (loginUsuario(user, senha)) {
                    printf("Login bem-sucedido!\n");
                    logado = 1;
                } else {
                    printf("Usuário ou senha incorretos.\n");
                }
                break;

            case 3:
                if (logado) {
                    alterarUsuario();
                } else {
                    printf("Você precisa fazer login primeiro!\n");
                }
                break;

            case 4:
                if (logado) {
                    excluirUsuario();
                } else {
                    printf("Você precisa fazer login primeiro!\n");
                }
                break;

            case 5:
                if (logado) {
                    listarUsuarios();
                } else {
                    printf("Você precisa fazer login primeiro!\n");
                }
                break;

            case 6:
                printf("Saindo...\n");
                break;

            default:
                printf("Opção inválida!\n");
                break;
        }

        if (opcao != 5) {
            sleep_seconds(2);
        }

    } while (opcao != 6);

    return 0;
}


