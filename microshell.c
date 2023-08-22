/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acouture <acouture@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/22 16:55:13 by acouture          #+#    #+#             */
/*   Updated: 2023/08/22 18:24:49 by acouture         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

typedef struct s_main {
    int in_fd;
    int pid_index;
    pid_t *pid;
}               t_main;

t_main *g() {
    static t_main data;
    return (&data);
}

int ft_strlen(char *s)
{
    int i = 0;
    while (s[i])
        i++;
    return (i);
}

void    print_error(char *s)
{
    if (s)
    {
        write(2, "error: cannot execute ", 22);
        write(2, s, ft_strlen(s));
        write(2, "\n", 1);
    }
}

void    ft_free2d(char **tab)
{
    int i = 0;
    if (!tab)
        return ;
    while (tab[i])
    {
        free(tab[i]);
        i++;
    }
    free(tab);
}

int ft_count_tokens(char **av)
{
    int i = 0;
    int tokens = 0;

    while (av[i])
    {
        if (strcmp(av[i], "|") == 0 || strcmp(av[i], ";") == 0)
            tokens++;
        i++;
    }
    return (tokens);
}

char *ft_strdup(char *s)
{
    int i = 0;
    char *result;

    if (!s)
        return (NULL);
    result = malloc(sizeof(char) * (ft_strlen(s) + 1));
    if (!result)
        return (NULL);
    while (s[i])
    {
        result[i] = s[i];
        i++;
    }
    result[i] = '\0';
    return (result);
}

bool ft_is_token(char *s)
{
    return (strcmp(s, "|") == 0 || strcmp(s, ";") == 0) ? false : true;
}

int init_pid(int token)
{
    g()->pid = malloc(sizeof(pid_t) * (token + 1));
    if (!g()->pid)
        return (-1);
    return (token);
}

void    close_and_wait()
{
    int i = 0;
    int status = 0;
    while (i < g()->pid_index)
    {
        waitpid(g()->pid[i], &status, 0);
        close(g()->in_fd);
        i++;
    }
    for (int i = 3; i < 200; i++) {
        close(i);
    }
}

int exec_cmd(char **cmd, bool multi, char **envp)
{
    int end[2];

    pipe(end);
    g()->pid[g()->pid_index] = fork();
    if (g()->pid[g()->pid_index] == 0)
    {
        if (multi)
            dup2(end[1], STDOUT_FILENO);
        else
            dup2(STDOUT_FILENO, STDOUT_FILENO);
        close(end[1]);
        close(end[0]);
        dup2(g()->in_fd, STDIN_FILENO);
        execve(cmd[0], cmd, envp);
        print_error(cmd[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        dup2(end[0], g()->in_fd);
        close(end[1]);
        return (0);
    }
}

int ft_cd(char **tab)
{
    int tab_len = 0;

    while (tab[tab_len])
        tab_len++;
    if (tab_len != 2)
    {
        write(2, "error: cd: bad arguments\n", 25);
        return (-1);
    }
    if (chdir(tab[1]) != 0)
    {
        write(2, "error: cd: cannot change directory to ", 39);
        write(2, tab[1], ft_strlen(tab[1]));
        write(2, "\n", 1);   
    }
    return (0);
}

int main(int ac, char **av, char **envp)
{
    int i = 1;
    int j;
    int cd_flag;
    int token = ft_count_tokens(av);
    token += 1;
    char **cmd_tab;
    int tab_index;

    (void)ac;
    cmd_tab = NULL;
    g()->pid_index = 0;
    init_pid(token);
    g()->in_fd = STDIN_FILENO;
    while (token > 0)
    {
        cd_flag = 0;
        j = i;
        tab_index = 0;
        while (av[i] && ft_is_token(av[i]))
            i++;
        cmd_tab = malloc(sizeof(char *) * (i + 1));
        if (!cmd_tab)
            return (1);
        while (j < i)
        {
            cmd_tab[tab_index] = ft_strdup(av[j]);
            if (strncmp(av[j], "cd", 2) == 0)
                cd_flag++;
            tab_index++;
            j++;
        }
        cmd_tab[tab_index] = NULL;
        if (cd_flag)
            ft_cd(cmd_tab);
        else if (av[i] && strcmp(av[i], "|") == 0)
        {
            if (exec_cmd(cmd_tab, true, envp) == -1)
                return (-1);
        }
        else
        {
            if (exec_cmd(cmd_tab, false, envp) == -1)
                return (-1);
        }
        i++;
        token--;
        ft_free2d(cmd_tab);
        g()->pid_index++;
    }
    close_and_wait();
    return (0);
}