require "erb"   

class Doxyfile

    def initialize(doxyfile, &block)
        @doxyfile = doxyfile.to_s
        @example_path = []
        @strip_from_path = []
        @input = []
        if block
            self.instance_exec(self, &block)
        end
    end

    attr_accessor :commit_number
    attr_accessor :version
    attr_accessor :use_mdfile_as_mainpage
    
    def input=(paths)
        @input = paths
    end

    def input
        @input.inject("") {|result, f| result << " #{f}" }
    end

    def example_path=(paths)
        @example_path = paths        
    end

    def example_path
        @example_path.inject("") {|result, f| result << " #{f}" }
    end

    def strip_from_path=(paths)
        @strip_from_path = paths
    end

    def strip_from_path
        @strip_from_path.inject("") {|result, f| result << " #{f}" }    
    end

    def to_s
        ERB.new(@doxyfile).result(binding)
    end

end

