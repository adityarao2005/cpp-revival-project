use clap::{Parser, Subcommand};

#[derive(Debug, Parser)]
#[command(name = "craft")]
#[command(about= "A CLI tool for C++ project and package management", long_about = None)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Debug, Subcommand)]
enum Commands {
    Init,
    Build,
    Run,
    Test,
    Export,
    Clean,
    Install,
}

fn main() {
    let args = Cli::parse();

    match args.command {
        Commands::Init => {
            craft::parse::init(craft::parse::InitArgs {
                project_name: None,
                project_type: None,
                cpp_version: None,
            })
            .unwrap();
        }
        Commands::Build => {
            craft::parse::build();
        }
        Commands::Run => {
            craft::parse::run();
        }
        Commands::Test => {
            craft::parse::test();
        }
        Commands::Export => {
            craft::parse::export();
        }
        Commands::Clean => {
            craft::parse::clean();
        }
        Commands::Install => {
            craft::parse::install();
        }
    }
}
