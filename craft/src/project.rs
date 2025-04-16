use std::fs;

use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ProjectFile {
    pub name: String,
    pub version: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub description: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub license: Option<String>,
    pub dependencies: Vec<dependency::Dependency>,
    pub project: Project,
}

pub fn read_project_file() -> Result<ProjectFile, std::io::Error> {
    // TODO: Return the project file if it exists
    // parses project.json
    todo!();
}

pub fn write_project_file(file: &ProjectFile) -> Result<(), std::io::Error> {
    let content = serde_json::to_string_pretty(file)?;
    fs::write(format!("./{}/project.json", file.name.clone()), content)?;

    Ok(())
}

pub mod dependency {
    use std::fmt::Debug;

    use serde::{Deserialize, Serialize};

    // The Dependency trait is used to define the common interface for all dependencies.
    #[derive(Debug, Serialize, Deserialize)]
    #[serde(rename_all = "camelCase")]
    pub enum Dependency {
        HeaderOnly {
            include_path: String,
            dependency_source: source::DependencySource,
        },
        Library {
            lib_path: String,
            lib_type: LibraryType,
            include_path: String,
            dependency_source: source::DependencySource,
        },
        SourceCode {
            src_path: String,
            build_command: String,
            lib_output: String,
            include_path: String,
            dependency_source: source::DependencySource,
        },
    }

    #[derive(Debug, Serialize, Deserialize)]
    #[serde(rename_all = "camelCase")]
    pub enum LibraryType {
        Static,
        Shared,
    }

    fn build_source_code(_src_path: &String, _build_command: &String, _lib_output: &String) {
        println!("Building source code dependency");

        // Here you would run the build command, for example:
        // TODO: implement this
        todo!();
    }

    impl Dependency {
        pub fn build(&self) {
            match self {
                Dependency::HeaderOnly { .. } => {
                    println!("Building header-only dependency");
                }
                Dependency::Library { lib_path, .. } => {
                    println!("Building library dependency at {}", lib_path);
                }
                Dependency::SourceCode {
                    src_path,
                    lib_output,
                    build_command,
                    ..
                } => {
                    println!("Building source code dependency at {}", src_path);
                    build_source_code(src_path, build_command, lib_output);
                }
            }
        }

        pub fn include_path(&self) -> Option<&String> {
            return match self {
                Dependency::HeaderOnly { include_path, .. } => Some(&include_path),
                Dependency::Library { include_path, .. } => Some(&include_path),
                Dependency::SourceCode { include_path, .. } => Some(&include_path),
            };
        }

        pub fn dependency_source(&self) -> &source::DependencySource {
            return match self {
                Dependency::HeaderOnly {
                    dependency_source, ..
                } => &(dependency_source),
                Dependency::Library {
                    dependency_source, ..
                } => &(dependency_source),
                Dependency::SourceCode {
                    dependency_source, ..
                } => &(dependency_source),
            };
        }
    }

    pub mod source {
        use std::fmt::Debug;

        use serde::{Deserialize, Serialize};

        #[derive(Debug, Serialize, Deserialize)]
        #[serde(rename_all = "camelCase")]
        pub enum DependencySource {
            GitSource {
                repo_url: String,
                #[serde(skip_serializing_if = "Option::is_none")]
                branch: Option<String>,
                #[serde(skip_serializing_if = "Option::is_none")]
                commit: Option<String>,
            },
            VcpkgSource {
                package_name: String,
                #[serde(skip_serializing_if = "Option::is_none")]
                version: Option<String>,
            },
            FolderSource {
                path: String,
            },
            SystemSource {
                lib_name: String,
            },
            ZipSource {
                file: String,
            },
        }

        impl DependencySource {
            pub fn extract_library(&self) {}
        }
    }
}

#[derive(Debug, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Project {
    pub test_dirs: Vec<String>,
    pub lib_dirs: Vec<String>,
    pub project_type: ProjectType,
    pub src_dirs: Vec<String>,
    pub src_files: Vec<String>,
    pub include_dirs: Vec<String>,
    pub include_files: Vec<String>,
    pub cpp_version: CPPVersion,
    pub build: BuildConfig,
}

#[derive(Debug, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct BuildConfig {
    pub build_dir: String,
    pub macros: Vec<String>,
    pub warning_level: WarningLevel,
}

#[derive(Debug, Serialize, Deserialize)]
pub enum WarningLevel {
    W4,
    W3,
    W2,
    W1,
    W0,
}

#[derive(Debug, Serialize, Deserialize)]
pub enum CPPVersion {
    CPP11,
    CPP14,
    CPP17,
    CPP20,
    CPP23,
}

#[derive(Debug, Serialize, Deserialize)]
pub enum ProjectType {
    Executable,
    StaticLibrary,
    SharedLibrary,
    HeaderOnlyLibrary,
}
