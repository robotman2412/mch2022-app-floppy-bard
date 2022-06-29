idf_component_register(
    SRCS
        "main.c"
        "artwork.c"
        "resources.c"
    INCLUDE_DIRS
        "." "include"
    EMBED_FILES ${project_dir}/main/resources/dust.png
)