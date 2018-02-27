package nlptools;

import edu.stanford.nlp.ie.machinereading.domains.ace.AceReader;
import edu.stanford.nlp.ling.CoreAnnotations;
import edu.stanford.nlp.pipeline.Annotation;
import edu.stanford.nlp.pipeline.StanfordCoreNLP;
import edu.stanford.nlp.util.CoreMap;
import edu.stanford.nlp.util.StringUtils;

import java.util.Properties;


public class AceEventAnnotations {
    public static void main(String[] args) throws Exception {
        String corpusHome = args[0];
//        String outputFilename = args[1];
//        BufferedWriter tokensWriter = new BufferedWriter(new FileWriter(outputFilename + ".tokens.txt"));
        Properties props = StringUtils.argsToProperties(args);
        AceReader r = new AceReader(new StanfordCoreNLP(props, false), false);
        Annotation docs = r.parse(corpusHome);
        for (CoreMap sentence: docs.get(CoreAnnotations.SentencesAnnotation.class)) {
            System.out.println(sentence.get(CoreAnnotations.TextAnnotation.class));
        }
        System.out.println("done");
    }
}

