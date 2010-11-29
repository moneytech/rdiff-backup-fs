#include "necessary.h"

int necessary_limit = DEFAULT_NECESSARY_LIMIT;

struct revision {
	char *name;
    char *file;
	struct node *tree;
};

typedef struct revision revision_t;

struct repository {
    char *name;
    struct revision *revisions;
};

typedef struct repository repository_t;

repository_t *repositories = NULL;
static struct stats root;

/* private functions' prototypes */

static struct node * get_revision_tree(char *repo, char *revision);

static int build_revision_tree(revision_t *, int, int);

static int find_snapshot(revision_t *, int, int);

static char * build_snapshot(revision_t *, int, int);

static int free_cache();

/*
 * checks whether repo with given name exists
 */
static int repo_exists(char *repo_name);

/*
 * checks whether revision in given repo exists
 */
static int revision_exists(char *repo_name, char *revision_name);

/* public functions */

int necessary_build(char *repo){

    #define necessary_build_finish(value) {                     \
        gstrdel(path);                                          \
        if (revs){                                              \
            for (i = 0; i < rev_count[0]; i++)                  \
                if (revs[i])                                    \
                    free(revs[i]);                              \
            free(revs);                                         \
        }                                                       \
        return value;                                           \
    }

    char *path = NULL;
    char **revs = NULL;
    int i = 0;

	if (gmstrcpy(&path, repo, "/rdiff-backup-data", 0) != 0)
		necessary_build_finish(-1);	
	if ((rev_count = single(int)) == NULL)
		necessary_build_finish(-1);
    if ((rev_count[0] = unzip_revs(path)) == -1)
    	necessary_build_finish(-1);
    if ((revs = calloc(rev_count[0], sizeof(char *))) == NULL)
    	necessary_build_finish(-1);
    if (get_revisions(rev_count[0], revs) == -1)
    	necessary_build_finish(-1);
    gstrsort(revs, rev_count[0]);
    if ((repositories = single(repository_t)) == NULL)
        necessary_build_finish(-1);
    repositories[0].revisions = calloc(rev_count[0], sizeof(revision_t));
    for (i = 0; i < rev_count[0]; i++) {
        repositories[0].revisions[i].name = get_revs_dir(revs[i]);
        gstrcpy(&repositories[0].revisions[i].file, revs[i]);
    }
    set_directory_stats(&root);
	necessary_build_finish(0);
}

int necessary_build_multi(int count, char **repo){
	return 0;
}

int necessary_get_file(char *repo, char *revision, char *internal, 
					   stats_t **stats){
#ifdef DEBUG
    printf("[necessary_get_file: checking file %s/%s/%s\n", repo, revision, internal);
#endif
    if (revision == NULL && (repo == NULL || repo_exists(repo))){
        *stats = &root;
        return 0;
    }
    else if (revision != NULL && internal == NULL){
        if (!revision_exists(repo, revision))
            return -1;
        else {
            *stats = &root;
            return 0;
        }
    }
    else{
        struct node *tree = get_revision_tree(repo, revision);
        if (!tree)
            return -1;
        return gtreeget(tree, internal, stats);
    }
}

char** necessary_get_children(char *repo, char *revision, char *internal){
    
    int i = 0;
    char **result = 0;

#ifdef DEBUG
    printf("[necessary_get_children: getting children of %s/%s/%s\n", repo, revision, internal);
#endif    
    if (revision == NULL){
        result = calloc(rev_count[0] + 1, sizeof(char *));
        for (i = 0; i < rev_count[0]; i++)
            gstrcpy(&result[i], repositories[0].revisions[i].name);
        return result;
    }
    struct node *tree = get_revision_tree(repo, revision);
    if (!tree)
        return NULL;
    return gtreecld(tree, internal);
}

/* private functions */

struct node * get_revision_tree(char *repo, char *rev){
    
    revision_t *revisions = NULL;
    int count = 0;
    int i = 0, j = 0;
    
    if (!repo) {
        revisions = repositories[0].revisions;
        count = rev_count[0];
    }
    else {
        for (i = 0; i < repo_count; i++)
            if (strcmp(repo, repos[i]) == 0){
                revisions = repositories[i].revisions;
                count = rev_count[i];
                break;
            }
        if (i == repo_count)
            // should never happen
            return NULL;
    }        
    for (j = 0; j < count; j++)
        if (strcmp(rev, revisions[j].name) == 0)
            break;
    if (j == count)
        // should never happen
        return NULL;
    if (!revisions[j].tree && build_revision_tree(revisions, rev_count[i], j))
        return NULL;
    return revisions[j].tree;
}

int build_revision_tree(revision_t *revisions, int count, int rev_index){
    
    int snapshot_index = 0;
    char *current_snapshot = NULL;
    
    if (free_cache())
        return -1;
    if ((snapshot_index = find_snapshot(revisions, count, rev_index)) == -1)
        return -1;
    if ((current_snapshot = build_snapshot(revisions, count, rev_index)) == NULL)
        return -1;
    return 0;
}

int find_snapshot(revision_t *revisions, int count, int rev_index){
    
    int snapshot_index = rev_index;
    char *ext = NULL;
    
    while (snapshot_index < count) {
        if ((ext = gpthext(revisions[snapshot_index].file)) == NULL)
            return -1;
        if (strcmp(ext, "snapshot") == 0){
            gstrdel(ext);
            break;
        }
        gstrdel(ext);
        snapshot_index++;
    }
    if (snapshot_index == count)
        // should never happen
        return -1;
    return snapshot_index;
    
};

char * build_snapshot(revision_t *revisions, int count, int rev_index) {
    return NULL;
};

int free_cache(){
    return 0;
}

int repo_exists(char *repo_name){
    return 0;
};

int revision_exists(char *repo_name, char *revision_name){
    
    revision_t *revisions = NULL;
    int i = 0;
    
    if (repo_name == NULL)
        revisions = repositories[0].revisions;
    else{
        // find repository
        return -1;
    };
    for (i = 0; i < rev_count[0]; i++)
        if (strcmp(revisions[i].name, revision_name) == 0)
            return 1;
    return 0;
    
};
