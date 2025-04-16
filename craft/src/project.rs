use std::fs;

use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub struct ProjectFile {
    pub name: String,
    pub version: String,
    pub description: Option<String>,
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
    pub enum Dependency {
        HeaderOnly(HeaderOnlyDependency),
        Library(LibraryDependency),
        SourceCode(SourceCodeDependency),
    }

    #[derive(Debug, Serialize, Deserialize)]
    pub struct HeaderOnlyDependency {
        pub include_path: String,
        pub dependency_source: source::DependencySource,
    }

    #[derive(Debug, Serialize, Deserialize)]
    pub struct LibraryDependency {
        pub lib_path: String,
        pub lib_type: LibraryType,
        pub include_path: String,
        pub dependency_source: source::DependencySource,
    }

    #[derive(Debug, Serialize, Deserialize)]
    pub struct SourceCodeDependency {
        pub src_path: String,
        pub build_command: String,
        pub lib_output: String,
        pub include_path: String,
        pub dependency_source: source::DependencySource,
    }

    #[derive(Debug, Serialize, Deserialize)]
    pub enum LibraryType {
        Static,
        Shared,
    }

    fn build_source_code(_src: &SourceCodeDependency) {
        println!("Building source code dependency");

        // Here you would run the build command, for example:
        // TODO: implement this
        todo!();
    }

    impl Dependency {
        pub fn build(&self) {
            match self {
                Dependency::HeaderOnly(..) => {
                    println!("Building header-only dependency");
                }
                Dependency::Library(dependency) => {
                    println!("Building library dependency at {}", dependency.lib_path);
                }
                Dependency::SourceCode(dependency) => {
                    println!("Building source code dependency at {}", dependency.src_path);
                    build_source_code(dependency);
                }
            }
        }

        pub fn include_path(&self) -> Option<&String> {
            return match self {
                Dependency::HeaderOnly(dependency) => Some(&(dependency.include_path)),
                Dependency::Library(dependency) => Some(&(dependency.include_path)),
                Dependency::SourceCode(dependency) => Some(&(dependency.include_path)),
            };
        }

        pub fn dependency_source(&self) -> &source::DependencySource {
            return match self {
                Dependency::HeaderOnly(dependency) => &(dependency.dependency_source),
                Dependency::Library(dependency) => &(dependency.dependency_source),
                Dependency::SourceCode(dependency) => &(dependency.dependency_source),
            };
        }
    }

    pub mod source {
        use std::fmt::Debug;

        use serde::{Deserialize, Serialize};

        #[derive(Debug, Serialize, Deserialize)]
        pub enum DependencySource {
            GitSource {
                repo_url: String,
                branch: Option<String>,
                commit: Option<String>,
            },
            VcpkgSource {
                package_name: String,
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
